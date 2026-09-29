total_size() {
  find "$1" -H -type f -print0 | xargs -0 stat -f '%z' | awk '{s+=$1} END {print s+0}'
}