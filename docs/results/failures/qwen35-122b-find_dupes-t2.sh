```bash
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

    # Find all regular files, compute MD5, format: hash  path
    # macOS: md5 -r outputs "hash  filename" (two spaces)
    find "$dir" -type f -exec sh -c '
        for f; do
            if hash=$(md5 -r "$f" 2>/dev/null); then
                # md5 -r output is "HASH  FILENAME", extract hash
                h=$(echo "$hash" | awk "{print \$1}")
                if [ -n "$h" ]; then
                    printf "%s  %s\n" "$h" "$f"
                fi
            fi
        done
    ' _ {} + | sort > "$tmpfile"

    # Group by hash, collect paths, filter groups with 2+ files, sort and print
    awk '
    {
        hash = $1
        path = $2
        for (i = 3; i <= NF; i++) path = path " " $i
        groups[hash] = groups[hash] (groups[hash] ? " " : "") path
        count[hash]++
    }
    END {
        for (hash in groups) {
            if (count[hash] > 1) {
                # Sort paths within the group
                n = split(groups[hash], paths, " ")
                # Simple bubble sort for paths (assuming no spaces in paths for simplicity in awk)
                # Actually, better to split properly: we stored full path in one field? No, awk split by space.
                # Reconstruct: we stored "hash  path" where path might have spaces.
                # The input to awk was "hash  path" where path is the rest of the line.
                # But awk $1 is hash, $2...$NF is path parts.
                # We need to reconstruct the original path.
                # Let's redo logic: store all lines per hash, then sort later.
                # Simpler: store in array as list of paths per hash.
                # But splitting by space breaks paths with spaces.
                # Fix: In the find loop, we ensured path is passed as argument, but here we read line.
                # Better approach: use null separator? Hard in awk.
                # Let's assume paths don't have newlines (true for find).
                # But spaces? We need to handle spaces.
                # Revised strategy:
                # The line format from find is "hash  path" (two spaces).
                # So we can split on first occurrence of "  " (two spaces).
                # But awk doesn't support multi-char separator easily in split.
                # Let's change the output format to use tab as separator.
            }
        }
    }
    '

    # Re-do with tab separator to handle spaces in filenames
    find "$dir" -type f -exec sh -c '
        for f; do
            if hash=$(md5 -r "$f" 2>/dev/null); then
                h=$(echo "$hash" | awk "{print \$1}")
                if [ -n "$h" ]; then
                    printf "%s\t%s\n" "$h" "$f"
                fi
            fi
        done
    ' _ {} + | sort > "$tmpfile"

    awk -F'\t' '
    {
        hash = $1
        path = $2
        # Store paths in array of arrays (simulated with string concatenation and delimiter)
        # We will store as: hash -> "path1\0path2\0..."
        # But simpler: store count and a list.
        if (hash in paths) {
            paths[hash] = paths[hash] "\n" path
        } else {
            paths[hash] = path
        }
        count[hash]++
    }
    END {
        for (hash in paths) {
            if (count[hash] > 1) {
                # Sort the paths for this hash
                n = split(paths[hash], arr, "\n")
                # Bubble sort arr
                for (i = 1; i < n; i++) {
                    for (j =