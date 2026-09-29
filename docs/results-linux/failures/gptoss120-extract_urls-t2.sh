extract_urls() {
    # Extract http:// or https:// URLs from stdin, one per line, preserving order.
    grep -oE 'https?://[^[:space:]]+' |
    # Trim trailing punctuation that is unlikely to be part of the URL.
    sed -e 's/[)\]\}>,.;:!?]*$//'
}