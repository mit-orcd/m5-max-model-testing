extract_urls() {
    grep -oE 'https?://[^[:space:]]+' | tr -d '[:punct:]' | grep -E '^https?://'
}