find_dupes() {
    local dir="$1"
    local tmpfile
    tmpfile=$(mktemp)
    trap "rm -f '$tmpfile'" RETURN

    # Find all regular files and compute their MD5 hashes
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        md5=$(md5 -r -- "$file" 2>/dev/null | awk '{print $1}')
        if [ -n "$md5" ]; then
            printf '%s\t%s\n' "$md5" "$file"
        fi
    done | sort -k1,1 -k2,2 > "$tmpfile"

    # Group files by MD5 and output groups with 2+ files
    awk -F'\t' '
        $1 != prev && NR > 1 {
            if (count >= 2) {
                for (i = 1; i <= count; i++) {
                    printf "%s", files[i]
                    if (i < count) printf " "
                }
                printf "\n"
            }
            count = 0
            delete files
        }
        {
            count++
            files[count] = $2
            prev = $1
        }
        END {
            if (count >= 2) {
                for (i = 1; i <= count; i++) {
                    printf "%s", files[i]
                    if (i < count) printf " "
                }
                printf "\n"
            }
        }
    ' "$tmpfile"
}