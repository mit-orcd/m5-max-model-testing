extract_urls() {
  grep -oP 'https?://[^\s"<>]+'
}