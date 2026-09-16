extract_urls() {
    grep -oE 'https?://[^[:space:]]+' | grep -vE '^[^[:space:]]*[^[:alnum:]/][^[:space:]]*$'
}