count_matches() {
  local pattern="$1"
  local file="$2"
  [[ -z "$file" ]] && file="-"
  grep -c -F -x "$pattern" "$file" 2>/dev/null || echo 0
}