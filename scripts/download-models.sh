#!/usr/bin/env bash
# Download models from Hugging Face. Usage: download-models.sh <listfile>
#
# Listfile format, one per line:  <hf-repo> [include-pattern]
#   - no pattern: plain snapshot into the HF cache (HF-format models, e.g. vLLM)
#   - with pattern: snapshot only matching files, then symlink them into
#     $MODELS_DIR (default ~/models) so llama.cpp can serve them by glob —
#     the pattern must match the "model" globs in bench.py's LINUX map.
# Sharded GGUFs: the pattern must match every shard (e.g. '*Q4_K_M*');
# llama-server loads shard 00001 and finds the rest beside it.
# A failing repo (404, auth, ...) is reported and skipped, not fatal.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LIST="${1:?usage: download-models.sh <listfile>}"
MODELS_DIR="${MODELS_DIR:-$HOME/models}"
FAILED=()

download_one() {
  local model="$1" pattern="${2:-}"
  if [[ -z "$pattern" ]]; then
    HF_HUB_ENABLE_HF_TRANSFER=1 "$ROOT/.venv/bin/python" -c "
from huggingface_hub import snapshot_download
p = snapshot_download('$model')
print('done', p)
"
    return
  fi
  mkdir -p "$MODELS_DIR"
  HF_HUB_ENABLE_HF_TRANSFER=1 "$ROOT/.venv/bin/python" - "$model" "$pattern" "$MODELS_DIR" <<'EOF'
import fnmatch
import sys
from pathlib import Path
from huggingface_hub import snapshot_download

repo, pattern, models_dir = sys.argv[1], sys.argv[2], Path(sys.argv[3])
snap = Path(snapshot_download(repo, allow_patterns=[pattern]))
linked = 0
for f in sorted(snap.rglob("*")):
    if f.is_file() and fnmatch.fnmatch(f.name, pattern):
        dest = models_dir / f.name
        if not dest.exists():
            dest.symlink_to(f)
        linked += 1
if linked == 0:
    print(f"WARNING: pattern {pattern!r} matched nothing in {repo}", file=sys.stderr)
    sys.exit(1)
print(f"done {snap} — linked {linked} file(s) into {models_dir}")
EOF
}

while read -r model pattern _; do
  [[ -z "${model:-}" || "${model:0:1}" == "#" ]] && continue
  echo "=== $model ${pattern:-}"
  if ! download_one "$model" "${pattern:-}"; then
    echo "FAILED: $model"
    FAILED+=("$model")
  fi
done < "$LIST"
if ((${#FAILED[@]})); then
  printf 'FAILED repos (%d): %s\n' "${#FAILED[@]}" "${FAILED[*]}" >&2
fi
echo "ALL DONE"
