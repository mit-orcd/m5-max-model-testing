find_dupes() {
    local dir="$1"
    local tmpfile=$(mktemp)
    find "$dir" -type f -exec md5 -r {} + | sort -k1,1 | awk '{printf "%s ", $2; for(i=3;i<=NF;i++) printf "%s", $i, (i==NF?ORS:FS)}' | sort -k1,1 > "$tmpfile"
    awk 'NF>=2 {print}' RS=' ' '\n' "$tmpfile" | sort -k1,1 | awk 'NF>=2' RS=' ' '\n' | tr '\n' ' ' | sed 's/  /\n/g' | sort -k1,1 | uniq -d | while read -r line; do
        echo "$line"
    done
    rm -f "$tmpfile"
}