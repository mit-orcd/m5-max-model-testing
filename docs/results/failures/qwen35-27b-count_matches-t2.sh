count_matches() {
  local pattern="$1"
  local file="$2"
  local count
  count=$(grep -F -c -- "$pattern" "$file" 2>/dev/null || echo 0)
  echo "$count"
}