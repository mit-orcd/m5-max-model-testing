extract_urls() {
  grep -Eo 'https?://[^[:space:]"]+'
}