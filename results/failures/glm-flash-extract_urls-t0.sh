extract_urls() {
  local input
  input=$(cat)
  echo "$input" | grep -oE 'https?://[^[:space:]"]+'
}