find_dupes() {
    local dir="$1"
    if [[ -z "$dir" ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    find "$dir" -type f -print0 | \
    while IFS= read -r -d '' file; do
        # Compute MD5 hash (BSD md5 -r)
        local hash
        hash=$(md5 -r "$file" | awk '{print $1}')
        printf '%s\t%s\n' "$hash" "$file"
    done | \
    sort -k1,1 -k2,2 | \
    awk -F'\t' '
        {
            if ($1 != curhash) {
                if (n > 1) {
                    out = paths[1]
                    for (i = 2; i <= n; i++) out = out " " paths[i]
                    print out
                }
                curhash = $1
                n = 0
            }
            n++
            paths[n] = $2
        }
        END {
            if (n > 1) {
                out = paths[1]
                for (i = 2; i <= n; i++) out = out " " paths[i]
                print out
            }
        }
    ' | sort -k1,1
}