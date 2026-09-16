find_dupes() {
    local dir="$1"
    if [[ -z "$dir" ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    if [[ ! -d "$dir" ]]; then
        echo "Error: Directory '$dir' not found" >&2
        return 1
    fi

    local tmpfile
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT

    # Find all regular files, compute MD5, and format as "hash  path"
    # macOS md5 outputs "MD5 (file) = hash", so we parse it.
    # We use -r to get just the hash and filename in a consistent format if available,
    # but standard BSD md5 output is "MD5 (filename) = hash".
    # To ensure portability and correct parsing on macOS:
    find "$dir" -type f -print0 2>/dev/null | while IFS= read -r -d '' file; do
        # Compute MD5
        hash=$(md5 -q "$file" 2>/dev/null)
        if [[ -n "$hash" ]]; then
            printf '%s  %s\n' "$hash" "$file"
        fi
    done > "$tmpfile"

    # Sort by hash, then by path
    # Then group by hash, collect paths, and print if count >= 2
    sort -k1,1 -k2 "$tmpfile" | awk '
    {
        hash = $1
        # Reconstruct path (in case path has spaces, though we used space separator in printf)
        # Since we printed "hash  path", $2 is the first part of path.
        # Better approach: use a delimiter that won't appear in paths, or handle carefully.
        # Let's re-parse: The line is "hash  path".
        # We can use substr to get the path starting after the hash and two spaces.
        path = substr($0, length($1) + 3)
        
        if (hash == prev_hash) {
            paths[hash] = paths[hash] " " path
            counts[hash]++
        } else {
            if (prev_hash != "" && counts[prev_hash] >= 2) {
                # Sort the paths for this group
                n = split(paths[prev_hash], arr, " ")
                # Simple bubble sort for small groups or use asorti if gawk, but we need POSIX awk
                # To keep it simple and robust, we will rely on the input being sorted by path within hash?
                # Actually, we sorted by hash then path, so paths for same hash are already sorted.
                # But we need to strip the leading space.
                print paths[prev_hash]
            }
            prev_hash = hash
            paths[hash] = path
            counts[hash] = 1
        }
    }
    END {
        if (prev_hash != "" && counts[prev_hash] >= 2) {
            print paths[prev_hash]
        }
    }
    '
}