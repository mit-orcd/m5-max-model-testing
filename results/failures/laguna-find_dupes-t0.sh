find_dupes() {
    local dir="$1"
    local tmpfile=$(mktemp)
    find "$dir" -type f -exec md5 -r {} \; | sort -k1,1 | awk '{print $1 " " $2}' > "$tmpfile"
    awk '{files[$1] = files[$1] " " $2} END {for (hash in files) {n = split(files[hash], arr, " "); if (n > 1) {delete arr[1]; cmd = "printf \"%s\""; for (i in arr) cmd = cmd " \"%s\""; cmd = cmd "\n"; n = 0; for (i in arr) {n++; if (n%2==0) {print arr[i]}} | cmd}}}' "$tmpfile" | while read -r line; do
        echo "$line" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//'
    done | sort -u | while read -r group; do
        count=$(echo "$group" | wc -w)
        if [ "$count" -ge 2 ]; then
            echo "$group"
        fi
    done
    rm -f "$tmpfile"
}