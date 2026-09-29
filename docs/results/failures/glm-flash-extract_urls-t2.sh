extract_urls() {
  local input="$((cat || true))"
  local URLs_REGEX='https?://[^\s<>"\'()]'
  printf '%s\n' $input | grep -oE "$URLS_REGEX" | sort -u
}