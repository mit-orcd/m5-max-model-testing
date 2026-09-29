extract_urls() {
  grep -oEh 'https?://[^ <>"]+'
}