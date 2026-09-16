find_dupes() {
    local search_dir="$1"
    find "$search_dir" -type f -exec md5 -r {} + | \
    sort | \
    awk '
    {
        hash = $1
        $1 = ""
        sub(/^ /, "", $0)
        paths[hash] = paths[hash] ? paths[hash] " " $0 : $0
        count[hash]++
    }
    END {
        for (h in count) {
            if (count[h] > 1) {
                print paths[h]
            }
        }
    }' | \
    while read -r line; do
        # Sort paths within the group
        echo "$line" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//'
    done | \
    sort
}