find_dupes() {
  local dir="$1"
  find "$dir" -type f -exec md5 -r {} + | awk '{print $1}' | sort | uniq -d | while read -r md5; do
    find "$dir" -type f -exec md5 -r {} + | awk -v md5="$md5" '$1 == md5 {print $2}' | sort
  done
}