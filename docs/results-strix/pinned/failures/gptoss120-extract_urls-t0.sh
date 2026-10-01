extract_urls() {
    local regex='https\?://[^[:space:]<>"]+'
    if (( $# )); then
        grep -oE "$regex" "$@"
    else
        grep -oE "$regex"
    fi
}