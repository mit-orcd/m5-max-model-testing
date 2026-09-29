extract_urls() {
    grep -oE 'https?://[^[:space:]]+' /dev/stdin
}