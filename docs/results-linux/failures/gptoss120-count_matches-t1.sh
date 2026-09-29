count_matches() {
    local pattern=$1
    local file=$2

    # Use grep with fixed‑string matching; suppress errors (e.g., missing file)
    # and fallback to 0 if grep fails.
    grep -F -c -- "$pattern" "$file" 2>/dev/null || echo 0
}