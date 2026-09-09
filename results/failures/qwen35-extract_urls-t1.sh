extract_urls() {
  grep -oE 'https?://[^[:space:]<>"'\'']+' | \
  sed -n 's/.*\(https\?:\/\/[^[:space:]<>"'\'']*\).*/\1/p'
}