function extract_urls() {
    sed -n '/https?:\/\/[^\s/]*[^\s]*/p'
}