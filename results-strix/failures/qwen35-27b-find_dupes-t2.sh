find_dupes() {
    local dir="$1"
    shift
    if [[ -z "$dir" || ! -d "$dir" ]]; then
        return 1
    fi

    # Create a temporary file to store hash:path pairs
    local tmpfile
    tmpfile=$(mktemp)
    trap "rm -f '$tmpfile'" EXIT

    # Find all regular files and compute their MD5 hashes
    # macOS 'find' outputs path, 'md5 -r' outputs "MD5 (path) = hash"
    # We use find to iterate and md5 -r to get hash and path
    while IFS= read -r -d '' file; do
        local hash_output
        hash_output=$(md5 -r "$file")
        # Parse md5 -r output: "MD5 (path) = hash"
        # Extract hash (last field) and the path (second field, handling spaces in path is tricky with md5 -r)
        # md5 -r format on macOS: "MD5 (filename) = hash" or "MD5 (path) = hash"
        # Let's extract the hash which is the last whitespace-delimited token, 
        # and the path is everything between the first '(' and the last ')'
        # Actually, md5 -r output: "MD5 (filepath) = hash"
        # We can extract hash easily. For path, we need to be careful with spaces.
        # A more robust way for macOS:
        # The format is strictly: "MD5 (path) = hash"
        # We can cut the hash, and reconstruct path or just rely on the fact that find passed us the file.
        local hash
        hash=$(echo "$hash_output" | awk '{print $NF}')
        # Re-verify the path to ensure we match the file we scanned
        # md5 -r might escape the path or not, but usually it prints the path as given.
        # To be safe against spaces in filenames, we rely on the 'file' variable from find.
        printf '%s\t%s\n' "$hash" "$file" >> "$tmpfile"
    done < <(find "$dir" -type f -print0)

    # Sort by hash, then by path to group duplicates
    # We use awk to group by hash, collect paths, and print if count > 1
    sort -t$'\t' -k1,1 -k2,2 "$tmpfile" | awk -F'\t' '
    {
        hash = $1
        path = $2
        if (hash != prev_hash) {
            if (prev_count > 1) {
                # Sort the paths for this group and print
                # Since input is sorted by path for same hash, we can just join
                print group
            }
            group = path
            prev_hash = hash
            prev_count = 1
        } else {
            group = group " " path
            prev_count++
        }
    }
    END {
        if (prev_count > 1) {
            print group
        }
    }
    ' | sort
}