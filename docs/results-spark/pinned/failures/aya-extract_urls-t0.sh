function extract_urls() {
    while IFS= read -r line; do
        urls=$(echo "$line" | perl -ne 'print "https://$1" if /(https?:\/\/[^\s]+)/')
        echo "$urls"
    done
}