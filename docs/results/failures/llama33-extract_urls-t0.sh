extract_urls() {
  grep -oE 'https?://[^[:space:]]+' | tr '\n' '\0' | xargs -0 -n1 echo
}