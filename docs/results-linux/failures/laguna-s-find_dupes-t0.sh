find_dupes() {
    local dir="$1"
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
                n = split(paths[h], arr, " ")
                # sort paths
                for (i = 1; i <= n; i++) {
                    for (j = i + 1; j <= n; j++) {
                        if (arr[i] > arr[j]) {
                            tmp = arr[i]
                            arr[i] = arr[j]
                            arr[j] = tmp
                        }
                    }
                }
                line = ""
                for (i = 1; i <= n; i++) {
                    line = line " " arr[i]
                }
                sub(/^ /, "", line)
                print line
            }
        }
    }' |
    sort
}