find_dupes() {
    local search_dir="$1"
    find "$search_dir" -type f -exec md5 -r {} + | sort | awk '
    {
        hash = $1
        $1 = ""
        sub(/^ /, "", $0)
        paths[hash] = paths[hash] ? paths[hash] " " $0 : $0
        count[hash]++
    }
    END {
        for (h in paths) {
            if (count[h] > 1) {
            split(paths[h], arr, " ")
            asort(arr)
            for (i=1; i<=length(arr); i++) {
                printf "%s%s", arr[i], (i == length(arr) ? "" : " ")
            }
            print ""
            }
        }
    }' | sort -k1,1
}