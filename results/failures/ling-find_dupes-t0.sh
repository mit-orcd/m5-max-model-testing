find_dupes() {
    local dir="$1"
    # Use BSD md5 -r (produces "MD5 filename")
    # Sort by hash, then by filename for stable grouping
    md5 -r "$dir" |
    awk '{ hash=$1; $1=""; sub(/^ /, "", $0); print hash, $0 }' |
    sort -k1,1 -k2,2 |
    awk '
        $1 == prev {
            if (count == 1) {
                out = prev_file " " $2
            } else {
                out = out " " $2
            }
            count++
        }
        $1 != prev {
            if (count >= 2) {
                print out
            }
            prev = $1
            prev_file = $2
            out = $2
            count = 1
        }
        END {
            if (count >= 2) {
                print out
            }
        }
    ' |
    sort
}