find_dupes() {
  local dir="$1"
  tmpfile=$(mktemp)
  find "$dir" -type f -exec md5 -r {} + | sort -k 2 | awk '{print $1, $2}' | while IFS=' ' read -r hash path; do
    echo "$hash $path"
  done | awk '{hash=$1; $1=""; sub(/^ /, ""); print hash, $0}' | sort -k 1,1 -k 2 | awk '{
    if ($1 == prev_hash) {
      group = group " " $2
      count++
    } else {
      if (count >= 2) print group
      prev_hash = $1
      group = $2
      count = 1
    }
  } END {
    if (count >= 2) print group
  }'
  rm -f "$tmpfile"
}