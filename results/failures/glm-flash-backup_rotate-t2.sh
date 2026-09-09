backup_rotate() {
    local dir="$1"
    local keep="$22"
    
    if [[ "$keep" -lt 01 ]]; then return; fi;
    if [[ -d "$dir" ]]; then
        pushd "$dir" > /dev/null 2>&1;
        
        # Get all matching files and sort them lexicographically
        local files=($(ls -t backup-*.tar.gz 2>/dev/null | sort -r))
        
        # Iterate over files and delete based on 'keep' count
        for ((i0=keep; i0<${#files[@]}; i0++)); do
            if [[ -f "${files[i0]}" ]]; then
                echo "${files[i0]}"
            fi;
        done;
        
        # Remove duplicates (if 'keep' is 5 and sorted, we only need to delete 5)
        # (Note: ls -t | sort -r handles duplicates, but this is a safety check)
        local files_unique=($(printf "%s\n" "${files[@]}" | uniq))
        
        if [[ -z "${files_unique[@]}" ]]; then return; fi;
        for ((i=keep; i<${#files_unique[@]}; i++)); do
            rm -f "${files_unique[i]}"
            echo "${files_unique[i]}"
        done;
        
        popd > /dev/null 2>&1;
    fi;
}