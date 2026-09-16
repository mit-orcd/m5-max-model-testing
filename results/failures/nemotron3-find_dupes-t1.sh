find_dupes() {
    local dir="$1"
    if [[ ! -d "$dir" ]]; then
        echo "Error: '$dir' is not a directory" >&2
        return 1
    fi

    # Find all regular files, compute MD5, sort by hash then path
    find "$dir" -type f -exec md5 -r {} \; |
    sort |
    # Group by MD5 hash, collect paths
    awk '
        {
            hash = $1
            path = substr($0, index($0, $2))
            groups[hash] = groups[hash] ? groups[hash] " " path : path
            count[hash]++
        }
        END {
            for (h in groups) {
                if (count[h] >= 2) {
                    # Split paths, sort them, rejoin
                    n = split(groups[h], paths, " ")
                    # Sort paths using system sort via temporary array trick
                    cmd = "printf \"%s\\n\", " groups[h] " | sort"
                    while ((cmd | getline line) > 0) {
                        sorted = sorted ? sorted " " line : line
                    }
                    close(cmd)
                    print sorted
                }
            }
        }
    ' |
    # Sort groups by first path in each group
    sort -k1,1
}