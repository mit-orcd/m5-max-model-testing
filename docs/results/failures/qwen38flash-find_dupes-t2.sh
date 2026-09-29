```bash
find_dupes() {
    local dir="$1"
    
    # Find all regular files, compute MD5, and store in a temp file
    # Format: <md5> <path>
    local tmpfile=$(mktemp)
    trap "rm -f '$tmpfile'" EXIT
    
    # Use find to get regular files, then use md5 -r to compute MD5
    # md5 -r outputs: <md5> <filename>
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        # Get MD5 using md5 -r (BSD)
        md5_output=$(md5 -r "$file" 2>/dev/null)
        if [ -n "$md5_output" ]; then
            # md5 -r output is: <hash> <path>
            # Extract hash (first field) and keep the path
            hash="${md5_output%% *}"
            echo "$hash $file"
        fi
    done > "$tmpfile"
    
    if [ ! -s "$tmpfile" ]; then
        return 0
    fi
    
    # Sort by hash first, then process groups
    # We need to group by MD5 hash, then for each group with 2+ files,
    # sort the paths and output them space-separated
    
    # Sort by hash (first field)
    sort -t ' ' -k1,1 "$tmpfile" | awk '
    {
        hash = $1
        # Remove the hash part to get the path
        path = substr($0, length(hash) + 2)
        
        if (hash == prev_hash) {
            # Append to current group
            groups[prev_hash] = groups[prev_hash] " " path
            count[prev_hash]++
        } else {
            # New hash
            if (prev_hash != "" && count[prev_hash] >= 2) {
                # Output the previous group if it had 2+ files
                # But we need to sort the paths within the group
                # Store for later processing
                hashes[++n] = prev_hash
            }
            prev_hash = hash
            groups[prev_hash] = path
            count[prev_hash] = 1
        }
    }
    END {
        if (prev_hash != "" && count[prev_hash] >= 2) {
            hashes[++n] = prev_hash
        }
        
        # Now for each hash with 2+ files, sort the paths and output
        for (i = 1; i <= n; i++) {
            h = hashes[i]
            # Split the paths and sort them
            # We need to collect paths, sort them, then output
            # This is tricky in awk. Let us use a different approach.
        }
    }
    ' 2>/dev/null
    
    # The awk approach above is getting complex. Let me use a simpler bash-based approach.
    
    # Let me redo this with a cleaner approach using sort and uniq
    rm -f "$tmpfile"
    
    # Recreate the temp file with hash and path
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        md5_output=$(md5 -r "$file" 2>/dev/null)
        if [ -n "$md5_output" ]; then
            hash="${md5_output%% *}"
            echo "$hash $file"
        fi
    done > "$tmpfile"
    
    if [ ! -s "$tmpfile" ]; then
        return 0
    fi
    
    # Get unique hashes that have more than one file
    # Count occurrences of each hash
    awk '{print $1}' "$tmpfile" | sort | uniq -c | awk '$1 >= 2 {print $2}' > /tmp/hash_counts_$$
    
    # For each duplicate hash, get all paths, sort them, and output
    while read -r hash; do
        # Get all paths for this hash, sort them
        paths=$(grep "^$hash " "$tmpfile" | awk '{ $1=""; sub(/^ /, ""); print }' | sort)
        
        # Count how many paths
        path_count=$(echo "$paths" | wc -l | tr -d ' ')
        
        if [ "$path_count" -ge 2 ]; then
            # Join paths with spaces
            joined=$(echo "$paths" | tr '\n' ' ' | sed 's/ *$//')
            echo "$joined"
        fi
    done < /tmp/hash