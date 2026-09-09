find_dupes() {
    local dir="${1:-.}"
    if [ ! -d "$dir" ]; then
        echo "Error: '$dir' is not a directory" >&2
        return 1
    fi

    # Find all regular files, compute MD5, store in associative array
    # Using BSD md5 -r output format: "MD5 (filename) = hash"
    # We need to handle filenames with spaces/newlines safely.
    
    declare -A hash_map
    
    # Use find with -exec to handle filenames safely
    # We collect: hash<TAB>filepath
    while IFS= read -r -d '' line; do
        # line format from md5 -r: "MD5 (filepath) = hash" or "MD5 filepath = hash"
        # macOS md5 -r output: "MD5 (path) = hash"
        # Extract hash: it's the part after "= "
        local hash="${line##*= }"
        hash="${hash%"${hash##*[![:space:]]}"}" # trim trailing space
        # Extract path: between "( " and " )" or handle if no parens
        local filepath="${line#MD5 (}"
        filepath="${filepath%%)" = *}"
        # If no parens (older format?), try:
        if [[ "$filepath" == *"MD5"* ]]; then
            # Fallback for non-paren format: "MD5 path = hash"
            filepath="${line#MD5 }"
            filepath="${filepath%% = *}"
        fi
        # Remove any residual spaces
        filepath="${filepath#"${filepath%%[![:space:]]*}"}"
        
        # Add to map: collect paths for each hash
        if [[ -n "$hash" ]]; then
            if [[ -v hash_map["$hash"] ]]; then
                hash_map["$hash"]+=$'\n'"$filepath"
            else
                hash_map["$hash"]="$filepath"
            fi
        fi
    done < <(find "$dir" -type f -print0 | xargs -0 md5 -r 2>/dev/null)

    # Process groups
    local output_lines=()
    for hash in "${!hash_map[@]}"; do
        local paths_str="${hash_map[$hash]}"
        # Count files
        local count=0
        while IFS= read -r p; do
            [ -z "$p" ] && continue
            ((count++))
        done <<< "$paths_str"
        
        if (( count >= 2 )); then
            # Sort paths within group
            local sorted_paths
            sorted_paths=$(echo "$paths_str" | sort)
            # Convert to space-separated string
            local line
            line=$(echo "$sorted_paths" | tr '\n' ' ' | sed 's/ *$//')
            output_lines+=("$line")
        fi
    done

    # Sort groups by first path
    if (( ${#output_lines[@]} > 0 )); then
        printf '%s\n' "${output_lines[@]}" | sort -t' ' -k1,1
    fi
}