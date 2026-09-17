find_dupes() {
    local dir="$1"
    if [ -z "$dir" ] || [ ! -d "$dir" ]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    # Find all regular files, compute MD5, sort by hash then path
    find "$dir" -type f | while IFS= read -r file; do
        hash=$(md5 -r "$file" 2>/dev/null | awk '{print $1}')
        if [ -n "$hash" ]; then
            echo "$hash $file"
        fi
    done | sort -k1,1 -k2 | awk '
        {
            hash = $1
            sub(/^[^ ]+ /, "", $0)
            path = $0
            if (hash == prev_hash) {
                paths = paths " " path
                count++
            } else {
                if (count >= 2) {
                    print paths
                }
                prev_hash = hash
                paths = path
                count = 1
            }
        }
        END {
            if (count >= 2) {
                print paths
            }
        }
    ' | sort
}