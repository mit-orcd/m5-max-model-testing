extract_urls() {
    grep -Eo 'https?://[^\s]+'
}