count_matches() {
  local pattern="$1"
  local file="$2"
  if [ ! -f "$file" ]; then
    echo 0
    return
  fi
  grep -c -F "$pattern" "$file" 2>/dev/null || echo 0
}