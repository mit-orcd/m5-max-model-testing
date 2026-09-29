extract_urls() {
  grep -oE 'https?://[^[:space:]"]+' | awk '{print $0}'
}