#!/bin/bash
# Decisive test: does Seed-OSS-36B honor thinking_budget=0 (direct answer)
# where enable_thinking=false failed? Runs on :8085 to stay clear of the queue.
set -u
cd "$(dirname "$0")/.."
PY=.venv/bin/python
PORT=8085

$PY -m mlx_lm.server --model mlx-community/Seed-OSS-36B-Instruct-4bit \
  --host 127.0.0.1 --port $PORT >/tmp/seed-test-server.log 2>&1 &
SRV=$!
trap 'kill $SRV 2>/dev/null' EXIT

for i in $(seq 1 120); do
  curl -sf --max-time 2 http://127.0.0.1:$PORT/v1/models >/dev/null 2>&1 && break
  kill -0 $SRV 2>/dev/null || { echo "SERVER DIED"; tail -5 /tmp/seed-test-server.log; exit 1; }
  sleep 5
done
echo "server up after ~$((i*5))s"

ask() {  # $1=label, $2=extra kwargs json
  local body
  body=$(cat <<EOF
{"model":"mlx-community/Seed-OSS-36B-Instruct-4bit",
 "messages":[{"role":"user","content":"Implement in C11: \`void reverse_string(char *s)\`. Reverses s in place.\nReply with only a C code block. No main function, no tests, no explanation."}],
 "max_tokens":1024,"temperature":0,"stream":false $2}
EOF
)
  local t0=$(python3 -c 'import time;print(time.time())')
  curl -s --max-time 300 http://127.0.0.1:$PORT/v1/chat/completions \
    -H 'Content-Type: application/json' -d "$body" > /tmp/seed-test-$1.json
  local t1=$(python3 -c 'import time;print(time.time())')
  $PY - "$1" "$t0" "$t1" <<'EOF'
import json, sys
label, t0, t1 = sys.argv[1], float(sys.argv[2]), float(sys.argv[3])
d = json.load(open(f"/tmp/seed-test-{label}.json"))
msg = d["choices"][0]["message"]["content"]
usage = d.get("usage", {})
think = msg.count("<seed:think>") + msg.count("<think>")
closed = msg.count("</seed:think>") + msg.count("</think>")
has_code = "```c" in msg
print(f"[{label}] {t1-t0:.0f}s  completion_tokens={usage.get('completion_tokens')}  "
      f"think_blocks={think} closed={closed} code_block={'YES' if has_code else 'NO'}")
print("  first 120 chars:", repr(msg[:120]))
EOF
}

ask budget0   ', "chat_template_kwargs": {"thinking_budget": 0}'
ask budget512 ', "chat_template_kwargs": {"thinking_budget": 512}'
echo "DONE"
