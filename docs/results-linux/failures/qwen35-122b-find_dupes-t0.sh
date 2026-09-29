find_dupes() {
    local dir="$1"
    local tmpfile
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT

    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | sort > "$tmpfile"

    awk '
    {
        hash = $1
        path = $2
        for (i = 3; i <= NF; i++) path = path " " $i
        if (hash in files) {
            files[hash] = files[hash] "\n" path
            count[hash]++
        } else {
            files[hash] = path
            count[hash] = 1
            order[++n] = hash
        }
    }
    END {
        for (i = 1; i <= n; i++) {
            h = order[i]
            if (count[h] > 1) {
                split(files[h], arr, "\n")
                # Sort paths within the group
                for (j = 1; j <= count[h]; j++) {
                    for (k = j + 1; k <= count[h]; k++) {
                        if (arr[j] > arr[k]) {
                            tmp = arr[j]
                            arr[j] = arr[k]
                            arr[k] = tmp
                        }
                    }
                }
                # Print sorted paths space-separated
                line = arr[1]
                for (j = 2; j <= count[h]; j++) {
                    line = line " " arr[j]
                }
                print line
            }
        }
    }
    ' "$tmpfile" | sort
}