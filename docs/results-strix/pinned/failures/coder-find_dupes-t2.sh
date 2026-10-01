find_dupes() {
    local dir="$1"
    local temp_file=$(mktemp)
    
    find "$dir" -type f -exec md5 -r {} \; | sort > "$temp_file"
    
    awk '{
        if ($1 == prev_hash) {
            if (first_path == "") {
                first_path = $2
            }
            paths[++count] = $2
        } else {
            if (count > 1) {
                for (i = 1; i <= count; i++) {
                    printf "%s ", paths[i]
                }
                printf "\n"
            }
            count = 1
            first_path = $2
            paths[1] = $2
            prev_hash = $1
        }
    }
    END {
        if (count > 1) {
            for (i = 1; i <= count; i++) {
                printf "%s ", paths[i]
            }
            printf "\n"
        }
    }' "$temp_file"
    
    rm -f "$temp_file"
}