find_dupes() {
  local dir="${1:-.}"
  local tmpfile
  tmpfile=$(mktemp)
  trap 'rm -f "$tmpfile"' EXIT

  find "$dir" -type f -exec md5 -r {} + 2>/dev/null | \
    awk '{
      hash = $1
      $1 = ""
      sub(/^[ \t]+/, "")
      path = $0
      files[hash] = files[hash] ? files[hash] " " path : path
      count[hash]++
    }
    END {
      for (h in count) {
        if (count[h] >= 2) {
          n = split(files[h], arr, " ")
          for (i = 1; i <= n; i++) {
            for (j = i + 1; j <= n; j++) {
              if (arr[i] > arr[j]) {
                tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp
              }
            }
          }
          line = arr[1]
          for (i = 2; i <= n; i++) {
            line = line " " arr[i]
          }
          print line
        }
      }
    }' | sort > "$tmpfile"

  cat "$tmpfile"
}