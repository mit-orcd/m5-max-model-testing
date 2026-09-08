#!/usr/bin/env bash
# Wait for the two download batches, then bench + eval all 6 comparison models.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"

echo "Waiting for downloads..."
while pgrep -f snapshot_download >/dev/null 2>&1; do sleep 60; done
echo "Downloads finished. Verifying..."
if find ~/.cache/huggingface/hub -name '*.incomplete' 2>/dev/null | grep -q .; then
  echo "WARNING: incomplete files remain" >&2
fi

for t in coder qwen35 gptoss gemma devstral aya; do
  "$ROOT/scripts/compare-model.sh" "$t" 3
done
echo "ALL COMPARISONS DONE"
