extract_urls() {
  grep -oE 'https?://[^ '"'"'<>]+' | awk '{ sub(/['"'"'<>]+$/, ""); print }'
}