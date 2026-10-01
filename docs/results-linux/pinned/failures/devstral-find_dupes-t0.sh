find_dupes() {
    dir="$1"
    find "$dir" -type f -exec md5 -r {} + | sort -k1,1 -k2,2 | uniq -w32 -d --all-repeated=prepend | awk '{print $2}' | sort -k1,1 | while read -r line; do
        paths=$(echo "$line" | sed 's/ .*//')
        count=$(echo "$line" | wc -w)
        if [ "$count" -ge 2 ]; then
            echo "$line"
        fi
    done
}