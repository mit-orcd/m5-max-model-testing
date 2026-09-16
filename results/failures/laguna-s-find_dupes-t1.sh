find_dupes() {
    local dir="$1"
    [[ -z "$dir" ]] && return 1
    [[ ! -d "$dir" ]] && return 1

    local tmp
    tmp=$(mktemp)
    trap 'rm -f "$tmp"' RETURN

    # Find regular files, compute md5, and store "md5\tpath" pairs
    find "$dir" -type f -exec md5 -r {} \; > "$tmp"

    # Sort by md5 (first field), then group identical md5s, and output groups with 2+ files
    sort "$tmp" | awk -F'\t' '
        {
            if (NF < 2) next
            md5 = $1
            path = $2
            gsub(/^[[:space:]]+/, "", md5)
            gsub(/^[[:space:]]+/, "", path)
            if (md5 != prev_md5) {
                if (count >= 2) {
                    for (i = 0; i < count; i++) print paths[i]
                    print ""
                }
                prev_md5 = md5
                count = 0
                delete paths
            }
            paths[count++] = path
        }
        END {
            if (count >= 2) {
                for (i = 0; i < count; i++) print paths[i]
                print ""
            }
        }
    ' | paste -sd ' ' - | sed '/^$/d' | sort
}