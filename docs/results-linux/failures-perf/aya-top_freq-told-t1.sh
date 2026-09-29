top_freq() {
    local file="$1"
    local tmpfile=$(mktemp)
    local count=$(wc -l < "$file")
    sort -V -t$'\t' -k2 -r -o "$tmpfile" < "$file"
    head -n 10 "$tmpfile" | awk -F'\t' '{print $2, $1}'
    rm "$tmpfile"
}