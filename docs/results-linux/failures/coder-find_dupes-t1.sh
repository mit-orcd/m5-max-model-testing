find_dupes() {
    local dir="$1"
    local temp_file
    temp_file=$(mktemp)
    
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | sort > "$temp_file"
    
    local current_md5=""
    local file_list=""
    local first_path=""
    
    while IFS= read -r line; do
        local md5=$(echo "$line" | cut -d' ' -f1)
        local path=$(echo "$line" | cut -d' ' -f2-)
        
        if [[ "$md5" == "$current_md5" ]]; then
            file_list="$file_list $path"
        else
            if [[ -n "$current_md5" && -n "$file_list" ]]; then
                # Print group if it has more than one file
                set -- $file_list
                if [[ $# -gt 1 ]]; then
                    # Sort paths and print
                    printf '%s\n' "$file_list" | sort | tr ' ' '\n' | tr '\n' ' ' | sed 's/ $//'
                fi
            fi
            current_md5="$md5"
            file_list="$path"
            first_path="$path"
        fi
    done < "$temp_file"
    
    # Handle the last group
    if [[ -n "$current_md5" && -n "$file_list" ]]; then
        set -- $file_list
        if [[ $# -gt 1 ]]; then
            printf '%s\n' "$file_list" | sort | tr ' ' '\n' | tr '\n' ' ' | sed 's/ $//'
        fi
    fi
    
    rm -f "$temp_file"
}