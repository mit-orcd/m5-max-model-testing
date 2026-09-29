find_dupes() {
    local dir="${1:-.}"
    
    # Find all regular files, compute md5, sort by md5 then path
    # Use find with -exec to run md5 -r on each file
    # md5 -r outputs: <hash>  <path>
    
    find "$dir" -type f -exec md5 -r {} + 2>/dev/null | \
    awk '{
        # md5 -r output format: hash  filename (two spaces)
        # Extract hash (first field) and filename (rest after first two spaces)
        hash = $1
        # Get the filename part: everything after "hash  "
        # The line is "hash  filename"
        # We can reconstruct filename by removing first 33 chars (32 hex + 2 spaces)
        # But safer to use substr or split
        # Actually, let's just use the fact that $1 is hash, and we need the rest
        # Remove hash and the two spaces
        sub(/^[^ ]+  /, "")
        filename = $0
        
        # Store mapping: hash -> list of filenames
        if (hash in files) {
            files[hash] = files[hash] "\n" filename
        } else {
            files[hash] = filename
            hashes[++count] = hash
        }
    }
    END {
        # For each hash, if multiple files, sort them and print
        for (i = 1; i <= count; i++) {
            h = hashes[i]
            n = split(files[h], arr, "\n")
            if (n < 2) continue
            
            # Sort the array of filenames
            for (j = 1; j <= n; j++) {
                for (k = j + 1; k <= n; k++) {
                    if (arr[j] > arr[k]) {
                        tmp = arr[j]
                        arr[j] = arr[k]
                        arr[k] = tmp
                    }
                }
            }
            
            # Build the output line
            line = arr[1]
            for (j = 2; j <= n; j++) {
                line = line " " arr[j]
            }
            
            # Store for final sorting by first path
            print line
        }
    }' | sort
}