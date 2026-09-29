find_dupes() {
    local dir="$1"
    local tmpfile=$(mktemp)
    
    # Find all regular files, compute MD5, store as "md5 path"
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | while IFS= read -r line; do
        # md5 -r outputs: <hash> <filename>
        # Extract hash and path
        local hash="${line%% *}"
        local path="${line#* }"
        echo "$hash $path"
    done > "$tmpfile"
    
    # Sort by hash, then by path
    sort -k1,1 -k2 "$tmpfile" > "${tmpfile}.sorted"
    
    # Group by hash, collect paths, print groups with 2+ files
    local current_hash=""
    local current_paths=""
    local count=0
    
    while IFS= read -r line; do
        local hash="${line%% *}"
        local path="${line#* }"
        
        if [[ "$hash" == "$current_hash" ]]; then
            current_paths="$current_paths $path"
            ((count++))
        else
            if [[ $count -ge 2 ]]; then
                # Sort the paths within the group
                local sorted_paths=$(echo "$current_paths" | tr ' ' '\n' | grep -v '^$' | sort | tr '\n' ' ' | sed 's/ $//')
                echo "$sorted_paths"
            fi
            current_hash="$hash"
            current_paths="$path"
            count=1
        fi
    done < "${tmpfile}.sorted"
    
    # Print the last group
    if [[ $count -ge 2 ]]; then
        local sorted_paths=$(echo "$current_paths" | tr ' ' '\n' | grep -v '^$' | sort | tr '\n' ' ' | sed 's/ $//')
        echo "$sorted_paths"
    fi
    
    rm -f "$tmpfile" "${tmpfile}.sorted"
}