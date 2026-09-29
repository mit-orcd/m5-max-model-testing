extract_urls() {
    grep -oE 'https?://[^[:space:]"]+' | tr -s '\n' '\n'
}