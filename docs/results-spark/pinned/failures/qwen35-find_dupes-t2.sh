find_dupes() {
  local dir="$1"
  [ -z "$dir" ] && dir="."
  
  find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | \
  awk '{
    hash = $1
    $1 = ""
    sub(/^ +/, "", $0)
    files[hash] = files[hash] (files[hash] ? "\n" : "") $0
    count[hash]++
  }
  END {
    for (h in count) {
      if (count[h] >= 2) {
        n = split(files[h], arr, "\n")
        for (i = 1; i <= n; i++) {
          for (j = i + 1; j <= n; j++) {
            if (arr[i] > arr[j]) {
              tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp
            }
          }
        }
        line = ""
        for (i = 1; i <= n; i++) {
          line = line (line ? " " : "") arr[i]
        }
        print line
      }
    }
  }' | sort
}