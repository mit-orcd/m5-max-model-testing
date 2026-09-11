extract_urls() {
    grep -oE 'https?://[^[:space:]]+' | sed 's/[[:space:]]*$//'
}