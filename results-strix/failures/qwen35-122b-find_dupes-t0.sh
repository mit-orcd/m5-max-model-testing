find_dupes() {
    local dir="$1"
    local tmpfile
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT

    # Find regular files, compute MD5, output "hash  path"
    # Use -exec with md5 -r (BSD) which outputs "hash  filename"
    # We need to handle filenames with spaces/newlines safely if possible, 
    # but standard find -exec + md5 -r is tricky with newlines. 
    # Assuming standard filenames without newlines for robustness in pure bash/POSIX.
    find "$dir" -type f -exec md5 -r {} \; | sort -k1,1 -k2,2 > "$tmpfile"

    local prev_hash=""
    local group=()
    local first_path=""

    while IFS= read -r line; do
        # Parse "hash  path" (md5 -r output has two spaces between hash and path)
        local hash="${line%%  *}"
        local path="${line#*  }"

        if [[ "$hash" != "$prev_hash" ]]; then
            # New hash group
            if [[ ${#group[@]} -ge 2 ]]; then
                # Print previous group if it had duplicates
                printf '%s\n' "${group[@]}" | sort | paste -sd ' ' -
            fi
            group=("$path")
            first_path="$path"
            prev_hash="$hash"
        else
            group+=("$path")
        fi
    done < "$tmpfile"

    # Handle the last group
    if [[ ${#group[@]} -ge 2 ]]; then
        printf '%s\n' "${group[@]}" | sort | paste -sd ' ' -
    fi
}