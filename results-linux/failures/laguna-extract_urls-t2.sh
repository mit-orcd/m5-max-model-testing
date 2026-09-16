extract_urls() {
    grep -oE 'https?://[^[:space:]]+' |
    while IFS= read -r url; do
        echo "$url"
    done
}