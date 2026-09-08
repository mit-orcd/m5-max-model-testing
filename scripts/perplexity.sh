#!/usr/bin/env bash
# Perplexity = canonical token-quality metric for quant comparison.
# Lower is better. Same corpus + seed => comparable across MLX models.
# Usage: scripts/perplexity.sh <model-id> [num_samples]
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MODEL="${1:?usage: perplexity.sh <model-id> [num_samples]}"
SAMPLES="${2:-50}"
exec "$ROOT/.venv/bin/mlx_lm.perplexity" \
  --model "$MODEL" \
  --num-samples "$SAMPLES" \
  --seed 0
