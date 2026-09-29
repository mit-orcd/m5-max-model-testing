count_matches() {
    local pattern=$1
    local file=$2

    # Validate arguments
    if [[ -z $pattern || -z $file ]]; then
        printf 'Usage: count_matches <pattern> <file>\n' >&2
        return 2
    fi

    # If file doesn't exist or isn't readable, treat as zero matches
    if [[ ! -r $file ]]; then
        printf '0\n'
        return 0
    fi

    # Count fixed‑string matches; grep -c prints 0 when there are none
    grep -F -c -- "$pattern" "$file" 2>/dev/null || printf '0\n'
}