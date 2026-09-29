find_dupes() {
    local dir="$1"
    local temp_file
    temp_file=$(mktemp)
    
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | sort | \
    awk '{
        if ($1 == prev_hash) {
            if (first_path == "") first_path = $2
            paths[++count] = $2
        } else {
            if (count >= 2) {
                for (i = 1; i <= count; i++) {
                    print paths[i]
                }
                print ""
            }
            prev_hash = $1
            count = 1
            first_path = $2
            paths[1] = $2
        }
    }
    END {
        if (count >= 2) {
            for (i = 1; i <= count; i++) {
                print paths[i]
            }
        }
    }' | sort | \
    awk '{
        if (NF == 0) {
            print ""
        } else {
            if (first_path == "") first_path = $0
            paths[++count] = $0
        }
    }
    END {
        if (count >= 2) {
            for (i = 1; i <= count; i++) {
                printf "%s ", paths[i]
            }
            print ""
        }
    }' > "$temp_file"
    
    sort -k1,1 "$temp_file" | tr ' ' '\n' | \
    awk '{
        if (NF == 0) {
            print ""
        } else {
            paths[++count] = $0
        }
    }
    END {
        if (count >= 2) {
            for (i = 1; i <= count; i++) {
                printf "%s ", paths[i]
            }
            print ""
        }
    }' | sort
    
    rm -f "$temp_file"
}