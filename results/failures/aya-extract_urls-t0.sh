function extract_urls() {
    while IFS= read -r line; do
        urls=$(echo "$line" | perl -wn0e 'print "http://$1" if /https?:\/\/[^\s/]+/i')
        echo "$urls"
    done
}