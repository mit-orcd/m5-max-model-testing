find_dupes() {
    local dir="$1"
    [ -d "$dir" ] || return 1

    # Collect regular files and compute MD5 hashes
    local tmpfile
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' RETURN

    # Use find to list regular files, then compute md5 for each
    while IFS= read -r -d '' file; do
        # md5 -r outputs "<hash>  <file>"
        md5 -r "$file" >> "$tmpfile" 2>/dev/null
    done < <(find "$dir" -type f -print0)

    # Sort by hash, then group identical hashes
    sort "$tmpfile" | awk '
        {
            hash = $1
            path = substr($0, index($0, $2))
            # Remove leading spaces from path
            sub(/^  /, "", path)
            if (hash == prev_hash) {
                group[++count] = path
            } else {
                if (count > 1) {
                    # Sort group and print
                    n = asorti(group, sorted)
                    for (i = 1; i <= n; i++) {
                        printf "%s%s", sorted[i], (i < n ? " " : "\n")
                    }
                }
                count = 1
                group[1] = path
            }
            prev_hash = hash
        }
        END {
            if (count > 1) {
                n = asorti(group, sorted)
                for (i = 1; i <= n; i++) {
                    printf "%s%s", sorted[i], (i < n ? " " : "\n")
                }
            }
        }
    '
}