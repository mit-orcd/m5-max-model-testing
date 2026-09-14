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
PPL_BIN="${LLAMA_PPL_BIN:-}"
if [[ -z "$PPL_BIN" ]]; then
  if command -v llama-perplexity >/dev/null 2>&1; then
    PPL_BIN=llama-perplexity
  else
    PPL_BIN="$HOME/llama.cpp/build/bin/llama-perplexity"
  fi
fi
[[ -s "$CORPUS" ]] || { echo "perplexity corpus missing: $CORPUS (run scripts/linux-setup.sh)" >&2; exit 1; }
exec "$PPL_BIN" -m "$GGUF" -f "$CORPUS" --chunks "$SAMPLES" -ngl 99 --seed 0
