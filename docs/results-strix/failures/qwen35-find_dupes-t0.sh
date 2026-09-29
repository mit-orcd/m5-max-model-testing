find_dupes() {
  local dir="$1"
  if [[ -z "$dir" ]]; then
    echo "Usage: find_dupes <dir>" >&2
    return 1
  fi

  # Find all regular files, compute MD5, sort by hash then path
  # macOS md5 -r outputs: "MD5 (file) = hash"
  find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
    hash=$(md5 -r "$file" | awk '{print $4}')
    printf '%s\t%s\n' "$hash" "$file"
  done | sort -t$'\t' -k1,1 -k2,2 | \
  awk -F'\t' '
  {
    if ($1 == prev_hash) {
      if (count == 1) {
        # Start of a new group, print previous single file as part of group
        group = prev_path "\t" $2
        count = 2
      } else {
        group = group "\t" $2
        count++
      }
    } else {
      # New hash, print previous group if it had 2+ files
      if (count >= 2) {
        print group
      }
      prev_hash = $1
      prev_path = $2
      group = $2
      count = 1
    }
  }
  END {
    if (count >= 2) {
      print group
    }
  }' | while IFS=$'\t' read -r line; do
    # Sort the paths within the group and print space-separated
    echo "$line" | tr '\t' '\n' | sort | paste -sd' ' -
  done | sort
}