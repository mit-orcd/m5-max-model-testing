find_dupes() {
  local dir="$1"
  if [[ -z "$dir" ]]; then
    echo "Usage: find_dupes <dir>" >&2
    return 1
  fi

  # Find all regular files, compute md5, sort by hash then path
  # macOS md5 -r outputs: "MD5 (file) = hash"
  # We need to parse this carefully.
  find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
    hash=$(md5 -r "$file" | awk '{print $NF}')
    printf '%s\t%s\n' "$hash" "$file"
  done | sort -t$'\t' -k1,1 -k2,2 | \
  awk -F'\t' '
  {
    hash = $1
    path = $2
    if (hash == prev_hash) {
      paths[hash] = paths[hash] " " path
      count[hash]++
    } else {
      if (count[hash] >= 2) {
        print paths[hash]
      }
      prev_hash = hash
      paths[hash] = path
      count[hash] = 1
    }
  }
  END {
    if (count[prev_hash] >= 2) {
      print paths[prev_hash]
    }
  }' | sort
}