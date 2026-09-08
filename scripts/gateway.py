#!/usr/bin/env python3
"""OpenAI-compatible preprocess + route gateway for Hermes."""

from __future__ import annotations

import argparse
import json
import logging
from typing import Any

import httpx
import uvicorn
from fastapi import FastAPI, Request, Response
from fastapi.responses import JSONResponse, StreamingResponse

MAIN_ID = "mlx-community/gpt-oss-20b-MXFP4-Q8"
FAST_ID = "mlx-community/Qwen3-8B-4bit"
MAIN_ALIASES = {MAIN_ID, "gpt-oss", "gpt-oss-20b", "qwen-27b", "qwen-main"}
FAST_ALIASES = {FAST_ID, "qwen-fast", "qwen-8b"}
MAIN_KV = 131072
MAIN_MAX_GEN = 16384
FAST_MAX_GEN = 4096
HARMONY_FINAL = "<|channel|>final<|message|>"
log = logging.getLogger("gateway")
app = FastAPI(title="Hermes MLX gateway")
settings: dict[str, str] = {
    "main": "http://127.0.0.1:8080",
    "fast": "http://127.0.0.1:8081",
}


def strip_harmony_text(text: str) -> str:
    if HARMONY_FINAL in text:
        return text.split(HARMONY_FINAL, 1)[1]
    if "<|channel|>" in text:
        return ""
    return text


def _rewrite_message_content(obj: dict[str, Any]) -> bool:
    """Strip harmony from message/delta content. True if the event should be forwarded."""
    changed = False
    for key in ("delta", "message"):
        block = obj.get(key)
        if not isinstance(block, dict):
            continue
        content = block.get("content")
        if isinstance(content, str) and content:
            stripped = strip_harmony_text(content)
            if stripped != content:
                block["content"] = stripped
                changed = True
    return changed


def strip_harmony_response(data: dict[str, Any]) -> dict[str, Any]:
    choices = data.get("choices")
    if not isinstance(choices, list):
        return data
    for choice in choices:
        if isinstance(choice, dict):
            _rewrite_message_content(choice)
    return data


class HarmonySseFilter:
    """Hold gpt-oss analysis tokens until the final channel, then pass the answer."""

    def __init__(self) -> None:
        self.linebuf = ""
        self.pending = ""
        self.mode = "unknown"  # unknown | analysis | final | passthrough

    def feed(self, chunk: bytes) -> bytes:
        self.linebuf += chunk.decode("utf-8", errors="replace")
        out: list[str] = []
        while "\n" in self.linebuf:
            line, self.linebuf = self.linebuf.split("\n", 1)
            rewritten = self._line(line)
            if rewritten is not None:
                out.append(rewritten + "\n")
        return "".join(out).encode()

    def flush(self) -> bytes:
        tail = ""
        if self.linebuf:
            rewritten = self._line(self.linebuf)
            self.linebuf = ""
            if rewritten is not None:
                tail = rewritten + "\n"
        return tail.encode()

    def _release(self, piece: str) -> str | None:
        if self.mode in ("passthrough", "final"):
            return piece
        self.pending += piece
        if HARMONY_FINAL in self.pending:
            self.mode = "final"
            out = self.pending.split(HARMONY_FINAL, 1)[1]
            self.pending = ""
            return out
        if "<|channel|>" in self.pending:
            self.mode = "analysis"
            return None
        if len(self.pending) >= 64:
            self.mode = "passthrough"
            out = self.pending
            self.pending = ""
            return out
        return None

    def _line(self, line: str) -> str | None:
        if not line.startswith("data:"):
            return line
        payload = line[5:].strip()
        if payload == "[DONE]":
            if self.mode == "unknown" and self.pending:
                synthetic = "data: " + json.dumps(
                    {"choices": [{"index": 0, "delta": {"content": self.pending}}]},
                    ensure_ascii=False,
                )
                self.pending = ""
                return synthetic + "\n" + line
            return line
        try:
            data = json.loads(payload)
        except json.JSONDecodeError:
            return line
        choices = data.get("choices")
        if not isinstance(choices, list) or not choices:
            return line
        choice = choices[0]
        if not isinstance(choice, dict):
            return line
        for key in ("delta", "message"):
            block = choice.get(key)
            if not isinstance(block, dict):
                continue
            content = block.get("content")
            if not isinstance(content, str):
                continue
            released = self._release(content)
            if released is None:
                block["content"] = ""
                if key == "delta" and not any(block.get(k) for k in block if k != "content"):
                    return None
            else:
                block["content"] = released
        return "data: " + json.dumps(data, ensure_ascii=False)


def _has_tools(body: dict[str, Any]) -> bool:
    tools = body.get("tools")
    return isinstance(tools, list) and len(tools) > 0


def _max_tokens(body: dict[str, Any]) -> int | None:
    for key in ("max_tokens", "max_completion_tokens"):
        value = body.get(key)
        if isinstance(value, int):
            return value
    return None


def route_backend(body: dict[str, Any]) -> str:
    model = str(body.get("model") or "").strip()
    if model in FAST_ALIASES:
        return "fast"
    if model in MAIN_ALIASES:
        return "main"
    if _has_tools(body):
        return "main"
    if body.get("response_format") or (_max_tokens(body) is not None and _max_tokens(body) <= 512):
        return "fast"
    return "main"


def _estimate_prompt_tokens(body: dict[str, Any]) -> int:
    blob = json.dumps(
        {"messages": body.get("messages"), "tools": body.get("tools")},
        ensure_ascii=False,
        default=str,
    )
    return max(1, (len(blob) + 2) // 3)


def _cap_generation(out: dict[str, Any], backend: str) -> None:
    hard_cap = MAIN_MAX_GEN if backend == "main" else FAST_MAX_GEN
    if backend == "main":
        room = MAIN_KV - _estimate_prompt_tokens(out) - 64
        hard_cap = max(1, min(hard_cap, room))
    requested = _max_tokens(out)
    capped = hard_cap if requested is None else min(requested, hard_cap)
    if "max_completion_tokens" in out and "max_tokens" not in out:
        out["max_completion_tokens"] = capped
    else:
        out["max_tokens"] = capped
        out.pop("max_completion_tokens", None)


def preprocess(body: dict[str, Any], backend: str) -> dict[str, Any]:
    out = dict(body)
    kwargs = out.get("chat_template_kwargs")
    if not isinstance(kwargs, dict):
        kwargs = {}
    else:
        kwargs = dict(kwargs)
    kwargs["enable_thinking"] = False
    out["chat_template_kwargs"] = kwargs

    extra = out.get("extra_body")
    if isinstance(extra, dict):
        extra = dict(extra)
        extra_kwargs = extra.get("chat_template_kwargs")
        if not isinstance(extra_kwargs, dict):
            extra_kwargs = {}
        else:
            extra_kwargs = dict(extra_kwargs)
        extra_kwargs["enable_thinking"] = False
        extra["chat_template_kwargs"] = extra_kwargs
        out["extra_body"] = extra

    if backend == "fast":
        out["model"] = FAST_ID
    else:
        out["model"] = MAIN_ID
        out.pop("response_format", None)
        if "reasoning_effort" not in out:
            out["reasoning_effort"] = "low"
    _cap_generation(out, backend)
    return out


def _backend_url(backend: str) -> str:
    return settings[backend].rstrip("/")


async def _models_payload() -> dict[str, Any]:
    seen: dict[str, dict[str, Any]] = {}
    async with httpx.AsyncClient(timeout=5.0) as client:
        for backend in ("main", "fast"):
            try:
                resp = await client.get(f"{_backend_url(backend)}/v1/models")
                resp.raise_for_status()
                for item in resp.json().get("data") or []:
                    mid = item.get("id")
                    if mid:
                        seen[mid] = item
            except Exception as exc:
                log.warning("backend %s /v1/models failed: %s", backend, exc)
    for alias, owned in (("gpt-oss", MAIN_ID), ("qwen-fast", FAST_ID)):
        seen[alias] = {"id": alias, "object": "model", "owned_by": owned}
    return {"object": "list", "data": list(seen.values())}


@app.get("/health")
async def health() -> JSONResponse:
    status: dict[str, Any] = {"ok": True, "backends": {}}
    async with httpx.AsyncClient(timeout=2.0) as client:
        for backend in ("main", "fast"):
            url = f"{_backend_url(backend)}/v1/models"
            try:
                resp = await client.get(url)
                status["backends"][backend] = {"up": resp.status_code < 400, "url": url}
                if resp.status_code >= 400:
                    status["ok"] = False
            except Exception as exc:
                status["ok"] = False
                status["backends"][backend] = {"up": False, "url": url, "error": str(exc)}
    return JSONResponse(status, status_code=200 if status["ok"] else 503)


@app.get("/v1/models")
async def list_models() -> JSONResponse:
    return JSONResponse(await _models_payload())


@app.api_route("/v1/chat/completions", methods=["POST"])
async def chat_completions(request: Request) -> Response:
    body = await request.json()
    backend = route_backend(body)
    payload = preprocess(body, backend)
    url = f"{_backend_url(backend)}/v1/chat/completions"
    headers = {"content-type": "application/json"}
    auth = request.headers.get("authorization")
    if auth:
        headers["authorization"] = auth
    log.info("route %s -> %s model=%s", body.get("model"), backend, payload.get("model"))

    timeout = httpx.Timeout(600.0, connect=10.0)
    if payload.get("stream"):
        client = httpx.AsyncClient(timeout=timeout)
        req = client.build_request("POST", url, json=payload, headers=headers)
        try:
            resp = await client.send(req, stream=True)
        except httpx.RequestError as exc:
            await client.aclose()
            return JSONResponse({"error": {"message": str(exc), "type": "gateway_error"}}, status_code=502)

        async def gen():
            filt = HarmonySseFilter() if backend == "main" else None
            try:
                async for chunk in resp.aiter_bytes():
                    if filt is None:
                        yield chunk
                        continue
                    out = filt.feed(chunk)
                    if out:
                        yield out
                if filt is not None:
                    tail = filt.flush()
                    if tail:
                        yield tail
            finally:
                await resp.aclose()
                await client.aclose()

        return StreamingResponse(
            gen(),
            status_code=resp.status_code,
            media_type=resp.headers.get("content-type", "text/event-stream"),
        )

    try:
        async with httpx.AsyncClient(timeout=timeout) as client:
            resp = await client.post(url, json=payload, headers=headers)
    except httpx.RequestError as exc:
        return JSONResponse({"error": {"message": str(exc), "type": "gateway_error"}}, status_code=502)
    content = resp.content
    ctype = resp.headers.get("content-type", "application/json")
    if backend == "main" and "application/json" in ctype:
        try:
            data = strip_harmony_response(resp.json())
            content = json.dumps(data).encode()
        except Exception:
            content = resp.content
    return Response(
        content=content,
        status_code=resp.status_code,
        media_type=ctype,
    )


def self_test() -> None:
    chat = {"model": MAIN_ID, "messages": [{"role": "user", "content": "hi"}]}
    assert route_backend(chat) == "main"
    assert preprocess(chat, "main")["model"] == MAIN_ID
    assert "response_format" not in preprocess({"response_format": {"type": "json_object"}, **chat}, "main")
    oversized = preprocess({**chat, "max_tokens": 65536}, "main")
    assert oversized["max_tokens"] <= MAIN_MAX_GEN

    title = {"model": "qwen-fast", "max_tokens": 64, "response_format": {"type": "json_object"}}
    assert route_backend(title) == "fast"
    fast = preprocess(title, "fast")
    assert fast["model"] == FAST_ID
    assert fast["response_format"]["type"] == "json_object"
    assert fast["chat_template_kwargs"]["enable_thinking"] is False

    tools = {"model": "unknown", "tools": [{"type": "function", "function": {"name": "x"}}]}
    assert route_backend(tools) == "main"
    main = preprocess(chat, "main")
    assert main["reasoning_effort"] == "low"

    raw = "<|channel|>analysis<|message|>think<|channel|>final<|message|>OK"
    assert strip_harmony_text(raw) == "OK"
    filt = HarmonySseFilter()
    chunk = filt.feed(
        b'data: {"choices":[{"delta":{"content":"<|channel|>analysis<|message|>x"}}]}\n'
        b'data: {"choices":[{"delta":{"content":"<|channel|>final<|message|>hi"}}]}\n'
        b"data: [DONE]\n"
    )
    assert b"hi" in chunk and b"analysis" not in chunk
    print("self-test ok")


def main() -> None:
    parser = argparse.ArgumentParser(description="Hermes MLX preprocess+route gateway")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=4000)
    parser.add_argument("--main", default="http://127.0.0.1:8080")
    parser.add_argument("--fast", default="http://127.0.0.1:8081")
    parser.add_argument("--test", action="store_true")
    args = parser.parse_args()
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
    if args.test:
        self_test()
        return
    settings["main"] = args.main
    settings["fast"] = args.fast
    uvicorn.run(app, host=args.host, port=args.port, log_level="info")


if __name__ == "__main__":
    main()
