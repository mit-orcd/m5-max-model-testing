extract_urls() {
    tr -d '\n' < /dev/stdin | grep -o 'http[s]*://[^"'\s]*'
}