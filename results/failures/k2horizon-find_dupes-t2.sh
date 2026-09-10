find_dupes() {
    local dir="$1"
    local md5file tmpfile
    tmpfile=$(mktemp)
    find "$dir" -type f -print0 | while IFS= read -r -d '' f; do
        md5=$(md5 -q "$f" 2>/dev/null) || continue
        printf '%s %s\n' "$md5" "$f" >> "$tmpfile"
    done
    sort -k1,1 "$tmpfile" | awk '
        $1 == prev {
            printf " %s", $2
        }
        $1 != prev {
            if (count > 1) printf "\n"
            count = 1
            printf "%s", $2
            prev = $1
        }
        END { if (count > 1) printf "\n" }
    ' | sed 's/^\([^ ]*\) /\1 /'
    rm -f "$tmpfile"
}