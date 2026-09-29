extract_urls() {
  grep -oP 'https?://[^[:space:]]+'
}