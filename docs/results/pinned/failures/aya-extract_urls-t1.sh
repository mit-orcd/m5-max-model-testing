function extract_urls() {
    while IFS= read -r line; do
        urls=$(echo "$line" | perl -ne 'print "$1\n" if /(https?|ftp):\/\/[^\s/]+/i')
        echo "$urls"
    done
}