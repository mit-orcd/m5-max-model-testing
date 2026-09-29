extract_urls() {
    grep -oE '(https?://[A-Za-z0-9._~:/?#\[\]@!$&'"'"'()*+,;=%{-]*)' 2>/dev/null
}