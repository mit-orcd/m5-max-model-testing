count_matches() {
  awk -v pattern="$1" 'index($0, pattern) { count++ } END { print count }' "$2"
}