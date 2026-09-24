find_dupes() {
    local dir="${1:-.}"

    command find "$dir" -type f -print0 \
        | while IFS= read -r -d '' file; do
              md5="$(md5 -r "$file" | awk '{print $1}')"
              printf '%s\t%s\n' "$md5" "$file"
          done \
        | sort -t $'\t' -k1,1 -k2 \
        | awk -F $'\t' '
            {
                md5 = $1
                # Extract path from everything after the first tab
                path = substr($0, index($0, "\t") + 1)
                if (md5 == prev) {
                    group = group " " path
                    count++
                } else {
                    if (count >= 2) print group
                    group = path
                    count = 1
                    prev = md5
                }
            }
            END {
                if (count >= 2) print group
            }
        '
}