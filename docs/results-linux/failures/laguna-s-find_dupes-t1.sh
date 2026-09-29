find_dupes() {
    local dir="$1"
    [ -n "$dir" ] || return 1
    find "$dir" -type f -exec md5 -r {} + 2>/dev/null |
        awk '{
            hash = $1
            rest = substr($0, index($0, $2))
            files[hash] = (hash in files ? files[hash] "\n" rest : rest)
            count[hash]++
        }
        END {
            for (h in count) {
                if (count[h] >= 2) {
                    cmd = "sort <<< \"" files[h] "\""
                    cmd | getline sorted
                    close(cmd)
                    gsub(/\n/, " ", sorted)
                    print sorted
                }
            }
        }' |
        sort
}