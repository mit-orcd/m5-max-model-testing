#!/usr/bin/env bash
# Perplexity = canonical token-quality metric for quant comparison.
# Lower is better. Same corpus + seed => comparable within one platform.
#
# Usage: scripts/perplexity.sh <target-or-model-id> [num_samples]
#
# macOS: mlx_lm.perplexity on the target's MLX model id.
# Linux: llama-perplexity on the target's GGUF ($MODELS_DIR), WikiText-2 raw
#        corpus at $PPL_CORPUS (default $MODELS_DIR/ppl/wiki.test.raw,
#        fetched by linux-setup.sh). --chunks approximates --num-samples.
#        Numbers are NOT comparable to the MLX ones (different quant + corpus
#        chunking); treat them as a fresh Linux baseline.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ARG="${1:?usage: perplexity.sh <target-or-model-id> [num_samples]}"
SAMPLES="${2:-50}"
PY="$ROOT/.venv/bin/python"

# Resolve a target name to its serve model; fall back to a literal model id.
MODEL="$("$PY" "$ROOT/scripts/serve.py" field "$ARG" serve_model 2>/dev/null || true)"
MODEL="${MODEL:-$ARG}"

if [[ "$(uname -s)" == "Darwin" ]]; then
  exec "$ROOT/.venv/bin/mlx_lm.perplexity" \
    --model "$MODEL" \
    --num-samples "$SAMPLES" \
    --seed 0
fi

# Linux: llama-perplexity on the GGUF. MODEL here is a glob under $MODELS_DIR.
GGUF="$("$PY" "$ROOT/scripts/serve.py" field "$ARG" gguf 2>/dev/null || true)"
GGUF="${GGUF:-$MODEL}"
MODELS_DIR="${MODELS_DIR:-$HOME/models}"
CORPUS="${PPL_CORPUS:-$MODELS_DIR/ppl/wiki.test.raw}"
RUNTIME="$("$PY" "$ROOT/scripts/serve.py" field "$ARG" runtime 2>/dev/null || true)"
PPL_BIN="${LLAMA_PPL_BIN:-}"
if [[ -z "$PPL_BIN" && "$RUNTIME" == "llamacpp-fork" && -n "${LLAMA_K2_SERVER_BIN:-}" ]]; then
  PPL_BIN="$(dirname "$LLAMA_K2_SERVER_BIN")/llama-perplexity"
fi
if [[ -z "$PPL_BIN" && -n "${LLAMA_SERVER_BIN:-}" ]]; then
  # sibling of the server binary
  PPL_BIN="$(dirname "$LLAMA_SERVER_BIN")/llama-perplexity"
fi
if [[ -z "$PPL_BIN" ]]; then
  for cand in llama-perplexity "$HOME/llama.cpp/build/bin/llama-perplexity" /home/root/llama.cpp/build/bin/llama-perplexity /home/llama.cpp/build/bin/llama-perplexity; do
    if [[ "$cand" == */* && -x "$cand" ]] || command -v "$cand" >/dev/null 2>&1; then
      PPL_BIN="$cand"
      break
    fi
  done
fi
[[ -s "$CORPUS" ]] || { echo "perplexity corpus missing: $CORPUS (run scripts/linux-setup.sh)" >&2; exit 1; }
export LD_LIBRARY_PATH="$(dirname "$PPL_BIN")${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec "$PPL_BIN" -m "$GGUF" -f "$CORPUS" --chunks "$SAMPLES" -ngl 99 --seed 0
