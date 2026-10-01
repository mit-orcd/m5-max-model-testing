find_dupes() {
  local dir="$1"
  if [[ -z "$dir" ]]; then
    echo "Usage: find_dupes <dir>" >&2
    return 1
  fi

  # Find all regular files, compute MD5, format as "md5 path"
  # Use -print0 and read -d '' to handle filenames with spaces/newlines safely
  find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
    # macOS uses 'md5 -r' for raw output (just the hash)
    hash=$(md5 -r "$file" 2>/dev/null | cut -d' ' -f1)
    if [[ -n "$hash" ]]; then
      printf '%s\t%s\n' "$hash" "$file"
    fi
  done | sort -t$'\t' -k1,1 -k2,2 | \
  awk -F'\t' '
    {
      if ($1 == prev_hash) {
        paths[NR] = $2
        count++
      } else {
        if (count > 1) {
          # Sort the paths for this group
          n = asort(paths, sorted_paths)
          line = ""
          for (i = 1; i <= n; i++) {
            line = line (i > 1 ? " " : "") sorted_paths[i]
          }
          print line
        }
        prev_hash = $1
        count = 1
        delete paths
        paths[NR] = $2
      }
    }
    END {
      if (count > 1) {
        n = asort(paths, sorted_paths)
        line = ""
        for (i = 1; i <= n; i++) {
          line = line (i > 1 ? " " : "") sorted_paths[i]
        }
        print line
      }
    }
  ' | sort
}