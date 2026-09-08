#!/usr/bin/env bash
# Download the 6-model comparison set. Usage: download-models.sh <listfile>
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LIST="${1:?usage: download-models.sh <listfile>}"
while read -r model; do
  [[ -z "$model" ]] && continue
  echo "=== $model"
  HF_HUB_ENABLE_HF_TRANSFER=1 "$ROOT/.venv/bin/python" -c "
from huggingface_hub import snapshot_download
p = snapshot_download('$model')
print('done', p)
"
done < "$LIST"
echo "ALL DONE"
