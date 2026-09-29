extract_urls() {
    grep -oE 'https?://[^[:space:]]+' | grep -oE 'https?://[^[:space:]]+'
}