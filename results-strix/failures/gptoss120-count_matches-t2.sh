count_matches() {
    local pattern=$1
    local file=$2

    # Validate arguments
    if [[ -z $pattern || -z $file ]]; then
        echo "Usage: count_matches <pattern> <file>" >&2
        return 1
    fi

    # If file doesn't exist or is not readable, treat as zero matches
    if [[ ! -r $file ]]; then
        echo 0
        return 0
    fi

    # Count fixed‑string matches; grep -c already prints 0 when there are none
    grep -F -c -- "$pattern" "$file" 2>/dev/null || echo 0
}