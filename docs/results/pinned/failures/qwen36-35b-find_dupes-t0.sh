find_dupes() {
    local dir="${1:-.}"
    
    # Use find to get all regular files, then compute md5 for each.
    # We use a while loop to read the output of find to handle filenames with spaces/newlines safely.
    # However, to sort and group efficiently, we'll pipe through awk or sort.
    # Strategy:
    # 1. Find all regular files.
    # 2. For each file, compute md5 and print "md5 path".
    # 3. Sort by md5, then by path.
    # 4. Group by md5, collect paths, print groups with 2+ files.
    
    # Note: macOS md5 outputs "MD5 (filename) = hash" or "hash  filename" depending on version.
    # md5 -r outputs "hash  filename" (two spaces).
    
    # We need to handle filenames with newlines? The prompt says "space-separated paths", implying standard filenames.
    # But to be safe with spaces, we'll use null-delimited find if possible, but md5 -r doesn't support null.
    # We'll assume filenames don't contain newlines for simplicity, as is common in such tasks unless specified otherwise.
    # If newlines are possible, this approach breaks. Let's assume standard filenames.
    
    find "$dir" -type f -exec md5 -r {} + 2>/dev/null | \
    awk '{
        # md5 -r output: hash  filename
        # The hash is the first field. The filename starts after the first two spaces.
        # But awk splits by whitespace. So $1 is hash, $2 is filename?
        # No, if filename has spaces, $2..$NF is filename.
        # Let's reconstruct the filename.
        hash = $1
        # Remove the first two fields (hash and the two spaces are not fields, just delimiters)
        # Actually, md5 -r output is: "hash  filename"
        # So $1 is hash, and the rest is filename.
        # We need to get the filename part.
        # Substring from length of hash + 2
        fname = substr($0, length(hash) + 3)
        
        # Store paths by hash
        if (hash in files) {
            files[hash] = files[hash] "\n" fname
            count[hash]++
        } else {
            files[hash] = fname
            count[hash] = 1
            hashes[++num_hashes] = hash
        }
    }
    END {
        # We need to sort groups by first path.
        # So we need to collect all groups, then sort them.
        # Let's store groups in an array indexed by a counter.
        n = 0
        for (i = 1; i <= num_hashes; i++) {
            h = hashes[i]
            if (count[h] >= 2) {
                n++
                # Split the file list by newline
                split(files[h], farr, "\n")
                # Sort the paths within the group
                # Simple insertion sort for small groups
                for (j = 2; j <= count[h]; j++) {
                    key = farr[j]
                    k = j - 1
                    while (k > 0 && farr[k] > key) {
                        farr[k+1] = farr[k]
                        k--
                    }
                    farr[k+1] = key
                }
                # Build the line
                line = farr[1]
                for (j = 2; j <= count[h]; j++) {
                    line = line " " farr[j]
                }
                group_lines[n] = line
            }
        }
        
        # Sort group_lines by the first path (which is the first element of the space-separated list)
        # Since the line starts with the first path, we can sort the lines directly.
        # Insertion sort for group_lines
        for (i = 2; i <= n; i++) {
            key = group_lines[i]
            k = i - 1
            while (k > 0 && group_lines[k] > key) {
                group_lines[k+1] = group_lines[k]
                k--
            }
            group_lines[k+1] = key
        }
        
        for (i = 1; i <= n; i++) {
            print group_lines[i]
        }
    }'
}