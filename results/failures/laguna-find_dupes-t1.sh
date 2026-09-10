find_dupes() {
    local dir="$1"
    local temp_file=$(mktemp)
    trap 'rm -f "$temp_file"' EXIT
    
    find "$dir" -type f -exec md5 -r {} \; | \
        awk '{print $1 " " $2}' | \
        sort -k1,1 -k2,2 | \
        awk '
            {
                if ($1 == prev_hash) {
                    files = files " " $2
                } else {
                    if (count >= 2) {
                        print prev_files
                    }
                    prev_hash = $1
                    prev_files = $2
                    count = 1
                }
            }
            END {
                if (count >= 2) {
                    print prev_files
                }
            }
        ' | \
        sort -k1,1
}