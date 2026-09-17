find_dupes() {
    local dir=$1
    if [[ -z $dir ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    find "$dir" -type f -print0 2>/dev/null |
    while IFS= read -r -d '' f; do
        md5 -r "$f"
    done |
    awk '{hash=$1; $1=""; sub(/^ /,""); print hash "\t" $0}' |
    sort -k1,1 -k2,2 |
    awk -F'\t' '
    {
        if ($1 != prev_hash) {
            if (count > 1) {
                printf "%s\n", group
            }
            prev_hash = $1
            count = 1
            group = $2
        } else {
            count++
            group = group " " $2
        }
    }
    END {
        if (count > 1) {
            printf "%s\n", group
        }
    }' |
    sort
}