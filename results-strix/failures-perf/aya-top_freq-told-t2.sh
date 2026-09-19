function top_freq() {
    local file="$1"
    local tmpfile=$(mktemp)
    local count=$(wc -l < "$file")
    local i=1
    while IFS= read -r line; do
        echo "$line" >> "$tmpfile"
        if [[ $i -eq $((count/10)) ]]; then
            break
        fi
        i=$((i+1))
    done < "$file"
    sort -nr -t' ' -k1,1 "$tmpfile" | awk -F' ' '{print $1, $2}' | head -n 10
}