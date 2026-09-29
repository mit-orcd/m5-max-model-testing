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
                cmd = "echo \"" paths[h] "\" | tr ' ' '\n' | sort | tr '\n' ' '"
                cmd | getline sorted
                close(cmd)
                sub(/ $/, "", sorted)
                print sorted
            }
        }
    }' | sort
}