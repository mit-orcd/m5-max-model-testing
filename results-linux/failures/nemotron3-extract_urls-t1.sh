extract_urls() {
    grep -oE 'https?://[^[:space:]]+' | tr '\n' '\n'
}