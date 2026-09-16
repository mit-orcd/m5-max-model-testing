find_dupes() {
    local dir="$1"
    [ -n "$dir" ] || return 1
    find "$dir" -type f -exec md5 -r {} + 2>/dev/null |
        awk '{
            hash = $1
            $1 = ""
            sub(/^ /, "")
            paths[hash] = paths[hash] " " $0
            count[hash]++
        }
        END {
            for (h in count) {
                if (count[h] > 1) {
                    split(paths[h], arr, " ")
                    n = asort(arr)
                    line = ""
                    for (i = 1; i <= n; i++) {
                        if (arr[i] != "") {
                            line = line " " arr[i]
                        }
                    }
                    sub(/^ /, "", line)
                    groups[line] = 1
                }
            }
            for (g in groups) print g
        }' |
        sort
}