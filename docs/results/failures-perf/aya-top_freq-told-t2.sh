top_freq() {
    local file="$1"
    local tmpfile=$(mktemp)
    local count=$(wc -l < "$file")
    sort -n -k1 -r < "$file" > "$tmpfile"
    local i=1
    while read -r line; do
        local val=$(echo "$line" | awk '{print $2}')
        local freq=$(grep -c "$val" "$file")
        echo "$freq $val"
        i=$((i-1))
        if [ "$i" -lt 10 ]; then
            :
        else
            break
        fi
    done < "$tmpfile"
    rm "$tmpfile"
}