count_matches() {
    local pattern=$1
    local file=$2

    # Use grep with fixed-string matching and count lines.
    # -F : treat pattern as a fixed string
    # -c : output the count of matching lines
    # -- : end of options (handles patterns beginning with '-')
    grep -F -c -- "$pattern" "$file" 2>/dev/null || echo 0
}