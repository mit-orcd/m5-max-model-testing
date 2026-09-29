extract_urls() {
    grep -oE 'https?://[^\s<>"{}|\\\^`\[\]]+' | sort -u
}