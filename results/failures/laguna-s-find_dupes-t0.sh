find_dupes() {
    local dir="$1"
    [ -d "$dir" ] || return 1
    local tmp
    tmp=$(mktemp)
    find "$dir" -type f -exec md5 -r {} \; | \
        awk '{
            hash=$1; $1=""; sub(/^ /, "");
            print hash "\t" $0
        }' | sort | \
        awk -F'\t' '
        {
            if (hash != $1) {
                if (count > 1) {
                    n = asort(files, sorted)
                    line = sorted[1]
                    for (i = 2; i <= n; i++) line = line " " sorted[i]
                    print line
                }
                hash = $1
                count = 0
                delete files
            }
            files[++count] = $2
        }
        END {
            if (count > 1) {
                n = asort(files, sorted)
                line = sorted[1]
                for (i = 2; i <= n; i++) line = line " " sorted[i]
                print line
            }
        }'
    rm -f "$tmp"
}