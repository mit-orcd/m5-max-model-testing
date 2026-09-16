extract_urls() {
    grep -oE 'https?://[^ ]+' | tr '\n' '\n'
}