find_dupes() {
    local dir="${1:-.}"
    local tmpfile
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT

    # Find regular files and compute MD5
    # Using find with -exec and md5 -r (BSD format: hash filename)
    # We assume filenames do not contain newlines (standard assumption unless specified otherwise)
    find "$dir" -type f -exec md5 -r {} + 2>/dev/null | \
        awk '{print $1, $2}' | \
        sort -k2 | \
        awk '{
            md5 = $1
            file = $2
            if (md5 == prev_md5) {
                group = group " " file
                count++
            } else {
                if (count >= 2) {
                    print group
                }
                prev_md5 = md5
                group = file
                count = 1
            }
        }
        END {
            if (count >= 2) {
                print group
            }
        }' | \
        sort -k1,1 | \
        while IFS= read -r line; do
            # Sort the files within each group
            echo "$line" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//'
            echo
        done
}