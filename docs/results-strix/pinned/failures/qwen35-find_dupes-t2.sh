find_dupes() {
    local dir="${1:-.}"
    if [[ ! -d "$dir" ]]; then
        return 1
    fi

    # Use find to get regular files, compute md5, sort by hash then path
    # macOS md5 -r outputs: "MD5 (file) = hash" or "MD5 (stdin) = hash"
    # We need to parse the hash and the filename.
    # Using -print0 and sort -z to handle spaces/newlines in filenames safely.

    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        # Compute MD5
        hash=$(md5 -r "$file" 2>/dev/null | awk '{print $NF}')
        if [[ -n "$hash" ]]; then
            printf '%s\t%s\n' "$hash" "$file"
        fi
    done | sort -t$'\t' -k1,1 -k2,2 | \
    awk -F'\t' '
    {
        hash = $1
        file = $2
        if (hash == prev_hash) {
            if (count == 1) {
                # Start of a new group, print previous single file as part of group
                # Actually, we need to buffer.
                # Let's restructure: collect files for current hash.
            }
            files[hash] = files[hash] " " file
            count[hash]++
        } else {
            # New hash, check if previous hash had duplicates
            if (count[prev_hash] > 1) {
                # Sort the files for the previous hash
                n = split(files[prev_hash], arr, " ")
                # Simple bubble sort for small groups or use asort if gawk, but we need portable
                # Since we need to output sorted paths, and we are processing sorted input,
                # the files for a hash are already sorted by path because of the outer sort.
                # So we just need to print them joined by space.
                print files[prev_hash]
            }
            prev_hash = hash
            files[hash] = file
            count[hash] = 1
        }
    }
    END {
        if (count[prev_hash] > 1) {
            print files[prev_hash]
        }
    }
    ' | sort
}