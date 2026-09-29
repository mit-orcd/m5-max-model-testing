extract_urls() {
    sed -n 's/.*\(http://\|https://\)[^ ]*.*/\1/p'
}