extract_urls() {
    grep -oE 'https?://[^[:space:]"'"'"'<>]+' | while read -r url; do
        printf '%s\n' "$url"
    done
}