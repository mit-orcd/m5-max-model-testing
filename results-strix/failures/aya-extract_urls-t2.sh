function extract_urls() {
    while IFS= read -r line; do
        urls=($(echo "$line" | sed -n 's#\b\(http://\|https://\)\([^\s]*\)\s*#\2#p'))
        for url in "${urls[@]}"; do
            echo "$url"
        done
    done
}