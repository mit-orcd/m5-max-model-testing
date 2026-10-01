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
        for (h in paths) {
            if (count[h] > 1) {
                n = split(paths[h], arr, " ")
                # Re-sort the paths
                for (i = 2; i <= n; i++) {
                    key = arr[i]
                    j = i - 1
                    while (j >= 1 && arr[j] > key) {
                        arr[j+1] = arr[j]
                        j--
                    }
                    arr[j+1] = key
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