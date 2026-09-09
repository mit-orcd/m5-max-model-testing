count_matches() {
  local pattern="$1"
  local file="$2"
  if [[ -z "$pattern" ]]; then
    echo 0
    return
  fi
  grep -F -c -- "$pattern" "$file" 2>/dev/null || echo 0
}