function extract_urls() {
    while IFS= read -r line; do
        urls=$(echo "$line" | perl -ne '/https?:\/\/[^\s"]+/g; print')
        echo "$urls"
    done
}