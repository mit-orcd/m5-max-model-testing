function extract_urls() {
    while IFS= read -r line; do
        urls=$(echo "$line" | perl -ne 'print "$1" if m{https?://[^\s"]+}g')
        echo "$urls"
    done
}