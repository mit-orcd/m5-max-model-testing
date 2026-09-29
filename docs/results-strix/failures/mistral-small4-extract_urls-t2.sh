extract_urls() {
  grep -oE 'https?://[^[:space:]"]+' | sed -e '/^http:\/\//!{/^https:\/\//!d}'
}