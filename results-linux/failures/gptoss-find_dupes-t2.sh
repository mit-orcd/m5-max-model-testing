find_dupes() {
    local dir="$1"
    if [[ -z "$dir" ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi
    if [[ ! -d "$dir" ]]; then
        echo "find_dupes: $dir: No such directory" >&2
        return 1
    fi

    local tmpfile
    tmpfile=$(mktemp -t md5paths.XXXXXX) || return 1

    # Generate checksum<tab>path lines
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        local checksum
        checksum=$(md5 -q "$file") || continue
        printf '%s\t%s\n' "$checksum" "$file"
    done > "$tmpfile"

    # Sort by checksum and path, group identical files, output groups
    sort -t $'\t' -k1,1 -k2,2 "$tmpfile" | awk -F'\t' '
        {
            if (checksum != $1) {
                if (checksum != "" && count > 1) {
                    output = paths[1]
                    for (i=2; i<=count; i++) output = output " " paths[i]
                    print output
                }
                checksum = $1
                count = 1
                paths[1] = $2
            } else {
                count++
                paths[count] = $2
            }
        }
        END {
            if (count > 1) {
                output = paths[1]
                for (i=2; i<=count; i++) output = output " " paths[i]
                print output
            }
        }' | sort

    rm -f "$tmpfile"
}