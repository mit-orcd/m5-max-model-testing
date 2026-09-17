extract_urls() {
    sed -n 's/.*\(https\?:\/\/[^[:space:]\"\'\'']*\).*/\1/p'
}