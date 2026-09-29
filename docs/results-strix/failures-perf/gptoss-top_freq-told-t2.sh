top_freq() {
    local file="$1"
    [[ -f "$file" ]] || { echo "File not found: $file" >&2; return 1; }
    sort "$file" | uniq -c | sort -k1,1nr | head -n10
}