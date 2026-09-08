extract_urls() {
    grep -oE 'https?://[^[:space:]"'"'"'<>]+'
}