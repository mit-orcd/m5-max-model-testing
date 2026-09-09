find_dupes() {
    local dir="${1:-.}"
    
    if [[ ! -d "$dir" ]]; then
        return 1
    fi

    # Use find with -print0 and read -d '' to handle filenames with spaces/newlines
    # macOS BSD find does not support -printf, so we use -exec sh -c
    # We collect name and md5 hash, then group by hash
    
    # Temporary file to store hash_path pairs
    local tmpfile=$(mktemp)
    local tmpgroup=$(mktemp)
    
    # Cleanup on exit
    trap "rm -f $tmpfile $tmpgroup" EXIT

    # Iterate over files, compute md5, store "hash  path"
    # Note: macOS md5 -r outputs "hash  filename" (two spaces)
    while IFS= read -r -d '' file; do
        # Get relative path or absolute? The prompt says "paths", usually relative to <dir> or absolute.
        # Standard behavior for such functions is to output the path as passed to find.
        # To ensure deterministic sorting, we store the path exactly as returned by find.
        hash=$(md5 -r "$file" | cut -d' ' -f1)
        printf '%s\t%s\n' "$hash" "$file" >> "$tmpfile"
    done < <(find "$dir" -type f -print0 2>/dev/null | sort -z)

    # Sort by hash, then by path
    sort -t$'\t' -k1,1 -k2,2 "$tmpfile" | \
    while IFS=$'\t' read -r hash path; do
        if [[ "$hash" == "$prev_hash" ]]; then
            echo "$path" >> "$tmpgroup"
        else
            if [[ -n "$tmpgroup" ]]; then
                # Process the group
                # Read lines, sort them, and print if count >= 2
                mapfile -t group_lines < "$tmpgroup"
                if [[ ${#group_lines[@]} -ge 2 ]]; then
                    # Sort the paths within the group and print space-separated
                    printf '%s\n' "${group_lines[@]}" | sort | xargs -n 1 | paste -sd ' ' -
                fi
            fi
            # Start new group
            echo "$path" > "$tmpgroup"
            prev_hash="$hash"
        fi
    done

    # Process the last group
    if [[ -n "$tmpgroup" ]]; then
        mapfile -t group_lines < "$tmpgroup"
        if [[ ${#group_lines[@]} -ge 2 ]]; then
            printf '%s\n' "${group_lines[@]}" | sort | xargs -n 1 | paste -sd ' ' -
        fi
    fi
    
    # Cleanup trap handles files, but we need to ensure variables are unset if loop breaks oddly
    rm -f "$tmpfile" "$tmpgroup" 2>/dev/null
}