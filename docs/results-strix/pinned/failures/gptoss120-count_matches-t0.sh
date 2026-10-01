count_matches() {
    local pattern=$1
    local file=$2

    # Ensure both arguments are provided
    if [[ -z $pattern || -z $file ]]; then
        echo "Usage: count_matches <pattern> <file>" >&2
        return 2
    fi

    # Use grep with fixed-string matching; -c prints the count (0 if none)
    grep -F -c -- "$pattern" "$file" 2>/dev/null || echo 0
}