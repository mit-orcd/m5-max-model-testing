extract_urls() {
    while IFS= read -r line; do
        echo "$line" | grep -o 'https?://\S+'
    done
}