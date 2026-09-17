function extract_urls {
    grep -o 'http[s]*://[^ ]*'
}