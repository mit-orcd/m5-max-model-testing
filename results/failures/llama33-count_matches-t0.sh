count_matches() {
  local pattern="$1"
  local file="$2"
  grep -F "$pattern" "$file" | wc -l
}