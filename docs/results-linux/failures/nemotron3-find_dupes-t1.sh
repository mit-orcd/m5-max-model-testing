find_dupes() {
    local dir="$1"
    local tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT

    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        md5 -r "$file" | awk -v f="$file" '{print $1 " " f}'
    done | sort | uniq -w32 --all-repeated=separate | cut -d' ' -f2- | while read -r line; do
        echo "$line"
    done | sort -t' ' -k1,1 | awk '
        { lines[$0] = 1 }
        END {
            n = asorti(lines, sorted)
            for (i = 1; i <= n; i++) {
                split(sorted[i], parts, " ")
                if (length(parts) >= 2) {
                    cmd = "printf \"%s\\n\", \"" sorted[i] "\" | sort"
                    while ((cmd | getline path) > 0) {
                        printf "%s ", path
                    }
                    close(cmd)
                    printf "\n"
                }
            }
        }
    '
}