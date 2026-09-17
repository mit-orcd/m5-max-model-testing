count_matches() {
    local pattern=$1 file=$2

    # If arguments are missing or file doesn't exist, output 0
    [[ -z $pattern || -z $file || ! -e $file ]] && { printf '0\n'; return; }

    # Count fixed‑string matches; if grep exits with status 1 (no matches), print 0
    grep -F -c -- "$pattern" "$file" 2>/dev/null || printf '0\n'
}