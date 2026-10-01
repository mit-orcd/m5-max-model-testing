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
                for (i = 1; i <= n; i++) {
                    if (arr[i] != "") {
                        sorted[i] = arr[i]
                    }
                }
                # Sort the paths
                for (i = 1; i <= n; i++) {
                    for (j = i + 1; j <= n; j++) {
                        if (sorted[i] > sorted[j]) {
                            tmp = sorted[i]
                            sorted[i] = sorted[j]
                            sorted[j] = tmp
                        }
                    }
                }
                line = ""
                for (i = 1; i <= n; i++) {
                    if (sorted[i] != "") {
                        line = line " " sorted[i]
                    }
                }
                sub(/^ /, "", line)
                print line
                delete sorted
            }
        }
    }' | sort
}