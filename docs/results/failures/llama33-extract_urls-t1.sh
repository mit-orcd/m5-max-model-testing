extract_urls() {
  grep -oE 'https?://[^[:space:]]+' | sed 's/[^/]*$//g' | grep -oE 'https?://[^[:space:]]+'
  while IFS= read -r line; do
    echo "$line" | grep -oE 'https?://[^[:space:]]+'
  done
}