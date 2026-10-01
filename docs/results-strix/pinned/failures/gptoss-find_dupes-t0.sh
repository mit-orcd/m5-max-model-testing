find_dupes() {
    local dir="$1"
    if [[ -z "$dir" ]]; then
        printf 'Usage: find_dupes <dir>\n' >&2
        return 1
    fi

    find "$dir" -type f -print0 | \
    while IFS= read -r -d '' file; do
        md5 -r "$file" | awk -v f="$file" -v OFS=$'\t' '{print $1, f}'
    done | \
    sort -t$'\t' -k1,1 -k2,2 | \
    awk -F$'\t' '
    {
        if (hash != $1) {
            if (count >= 2) {
                print group
            }
            hash = $1
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
    ' | \
    sort -k1,1
}