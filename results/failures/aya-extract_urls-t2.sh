function extract_urls() {
    while IFS= read -r line; do
        urls=$(echo "$line" | tr -d ' ' | grep -o 'https?://[^\s/]*')
        for url in $urls; do
            echo "$url"
        done
    done
}