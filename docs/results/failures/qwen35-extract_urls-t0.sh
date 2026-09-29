extract_urls() {
  grep -oE 'https?://[^[:space:]"<>'"]+' | while read -r url; do
    echo "$url"
  done
}