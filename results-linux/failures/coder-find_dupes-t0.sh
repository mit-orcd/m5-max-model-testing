find_dupes() {
    local dir="$1"
    local temp_file
    temp_file=$(mktemp)
    
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | sort > "$temp_file"
    
    awk '{
        if ($0 == prev) {
            if (first == "") first = prev_file
            dupes = dupes " " $2
        } else {
            if (first != "") {
                if (dupes != "") {
                    printf "%s%s\n", first, dupes
                }
                first = ""
                dupes = ""
            }
            prev = $0
            prev_file = $2
        }
    }
    END {
        if (first != "") {
            if (dupes != "") {
                printf "%s%s\n", first, dupes
            }
        }
    }' "$temp_file" | sort
    
    rm -f "$temp_file"
}