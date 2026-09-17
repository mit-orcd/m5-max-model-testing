find_dupes() {
    local dir="$1"
    local tmp=$(mktemp -t find_dupes.XXXXXX)
    local tmp2=$(mktemp -t find_dupes2.XXXXXX)
    trap "rm -f $tmp $tmp2" EXIT

    # Find all regular files and compute MD5
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | \
        awk '{print $1, $2}' | \
        sort -k1,1 -k2,2 > "$tmp"

    # Extract groups with 2+ files
    awk '
        $1 != prev && NR > 1 {
            if (count >= 2) {
                for (i = 1; i <= count; i++) {
                    print files[i]
                }
            }
            count = 0
        }
        {
            count++
            files[count] = $2
            prev = $1
        }
        END {
            if (count >= 2) {
                for (i = 1; i <= count; i++) {
                    print files[i]
                }
            }
        }
    ' "$tmp" | sort -k1,1 | \
    awk '
        BEGIN { first = 1 }
        {
            if (first) {
                printf "%s", $0
                first = 0
            } else {
                printf " %s", $0
            }
        }
        /^$/ { next }
        NF && !first || /^$/ {
            print ""
            first = 1
            printf "%s", $0
        }
    ' > "$tmp2"

    # Handle last group
    awk '
        BEGIN { first = 1 }
        {
            if (first) {
                printf "%s", $0
                first = 0
            } else {
                printf " %s", $0
            }
        }
    ' "$tmp2"

    rm -f "$tmp" "$tmp2"
    trap - EXIT
}