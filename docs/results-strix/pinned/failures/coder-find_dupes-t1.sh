find_dupes() {
    local dir="$1"
    local temp_file
    temp_file=$(mktemp)
    
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | sort > "$temp_file"
    
    awk '{
        if ($0 == prev) {
            if (first == "") first = prev_file
            files = files " " $2
        } else {
            if (first != "") {
                if (files != "") {
                    cmd = "echo " first files " | sort"
                    cmd | getline sorted
                    close(cmd)
                    print sorted
                }
                first = ""
                files = ""
            }
            prev = $0
            prev_file = $2
        }
    }
    END {
        if (first != "") {
            if (files != "") {
                cmd = "echo " first files " | sort"
                cmd | getline sorted
                close(cmd)
                print sorted
            }
        }
    }' "$temp_file"
    
    rm -f "$temp_file"
}