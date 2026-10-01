find_dupes() {
    local dir="$1"
    if [[ -z "$dir" ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        md5 -r "$file" | awk '
            { hash=$1; sub(/^([a-f0-9]+) +/, "", $0); print hash "\t" $0 }
        '
    done | sort -k1,1 -k2,2 | awk -F'\t' '
        {
            if (prev != $1) {
                if (count >= 2) print group
                prev = $1
                group = $2
                count = 1
            } else {
                group = group " " $2
                count++
            }
        }
        END {
            if (count >= 2) print group
        }
    ' | sort -k1,1
}