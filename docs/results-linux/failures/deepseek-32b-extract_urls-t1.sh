extract_urls() {
    grep -o -E 'https?://[^ ]+'
}