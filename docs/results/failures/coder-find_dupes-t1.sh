find_dupes() {
    local dir="$1"
    local -A hashes
    local -a paths
    local -a sorted_paths
    local -a groups
    local -a sorted_groups
    local -a sorted_paths

    # Find all regular files and calculate md5 hashes
    while IFS= read -r -d '' file; do
        # Use md5 -r to get hash
        local hash=$(md5 -r "$file" 2>/dev/null | awk '{print $1}')
        if [[ -n "$hash" ]]; then
            hashes[$hash="$hash"]+=" $file"
        fi
    done < <(find "$dir" -type f -print0)

    # Find duplicates and sort paths in each group
    for hash in "${!hashes[@]}"; do
        local paths_list="${hashes[$hash]}"
        if [[ "$paths_list" =~ [[:space:]$'\n'] ]]; then
            # Sort the paths and sort them lexicographically
            IFS=' ' read -r -a sorted_paths <<< "$(echo "$paths_list" | tr ' ' '\n' | sort)"
            if (( ${#sorted_paths[@]} > 1 )); then
                # Print full path list only if there are duplicates
                printf "%s\n" "${sorted_paths[*]}"
            endif
        endif
    done | sort
}