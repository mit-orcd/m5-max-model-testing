find_dupes() {
  local dir="$1"
  if [[ -z "$dir" ]]; then
    echo "Usage: find_dupes <dir>" >&2
    return 1
  fi

  find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
    hash=$(md5 -q "$file")
    printf '%s\t%s\n' "$hash" "$file"
  done | sort -k1,1 -k2,2 | awk -F'\t' '
    BEGIN{prev=""; paths=""; count=0}
    {
      if ($1 != prev) {
        if (count>1) print paths
        prev=$1; paths=$2; count=1
      } else {
        paths = paths " " $2; count++
      }
    }
    END{ if (count>1) print paths }
  ' | sort -k1,1
}