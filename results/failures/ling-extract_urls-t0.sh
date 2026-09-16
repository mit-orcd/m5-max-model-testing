extract_urls() {
    grep -oP 'https?://\S+'
}