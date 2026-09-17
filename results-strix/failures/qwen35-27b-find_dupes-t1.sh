find_dupes() {
    local dir="$1"
    [[ -z "$dir" ]] && return 1

    # Create temp file for hash:path pairs
    local tmpfile
    tmpfile=$(mktemp)
    trap "rm -f '$tmpfile'" EXIT

    # Find all regular files and compute MD5
    # BSD md5 -r outputs: "MD5 (filename) = hash" or "MD5 (filename) = hash"
    # We use -r for recursive reading, but find handles recursion.
    # BSD md5 -q is quiet, -r is for reading from stdin or file list? 
    # Actually on macOS: `md5 -q <file>` prints hash. `md5 -r` is not standard for reading filenames.
    # Standard macOS md5: `md5 file` -> "MD5 (file) = hash".
    # We need to parse this.
    
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        hash=$(md5 -q "$file" | awk '{print $1}')
        printf '%s\t%s\n' "$hash" "$file"
    done > "$tmpfile"

    # Sort by hash then path, then process groups
    sort -t$'\t' -k1,1 -k2,2 "$tmpfile" | \
    awk -F'\t' '
    {
        hash = $1
        path = $2
        if (hash == prev_hash) {
            groups[hash] = groups[hash] " " path
            counts[hash]++
        } else {
            # Output previous group if count >= 2
            if (counts[prev_hash] >= 2) {
                # Split group into array, sort paths, join
                n = split(groups[prev_hash], arr, " ")
                # Skip first empty element if any (due to leading space logic)
                # Actually groups[hash] starts with space, so arr[1] is empty
                # Let's rebuild logic:
            }
            prev_hash = hash
            groups[hash] = path
            counts[hash] = 1
        }
    }
    END {
        if (counts[prev_hash] >= 2) {
            print groups[prev_hash]
        }
    }
    ' | while read -r line; do
        # Sort the paths within the line
        echo "$line" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//'
    done | sort
}