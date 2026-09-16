find_dupes() {
    local dir="$1"
    local temp_file
    temp_file=$(mktemp)
    
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | \
    sort | \
    awk '{
        if ($1 == prev_hash) {
            if (first == "") first = prev_path
            paths = paths " " $2
        } else {
            if (first != "" && prev_hash != "") {
                printf "%s%s\n", first, paths
            }
            first = $2
            paths = " " $2
            prev_hash = $1
        }
        prev_path = $2
    }
    END {
        if (first != "" && prev_hash != "") {
            printf "%s%s\n", first, paths
        }
    }' | \
    sort | \
    sed 's/^[ ]*//'
    
    rm -f "$temp_file"
}