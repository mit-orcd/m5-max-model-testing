extract_urls() {
    # Extract http:// or https:// URLs from stdin, one per line, preserving order.
    # Uses grep to find non‑space sequences that start with the scheme.
    grep -oE 'https?://[^[:space:]]+'
}