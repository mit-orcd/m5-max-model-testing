find_dupes() {
    local dir="$1"
    if [ -z "$dir" ]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    # Find all regular files, compute md5, group by hash, filter groups with 2+ files
    # Use find to get files, then md5 -r to get hash, then process with awk/sort
    
    # Step 1: Get all regular files recursively
    # Step 2: Compute md5 for each
    # Step 3: Group by md5, collect paths
    # Step 4: Filter groups with >= 2 files
    # Step 5: Sort paths within each group, sort groups by first path
    
    # We'll use a temporary approach with associative arrays or sort+awk
    
    # Using find + md5 -r + sort + awk
    # md5 -r outputs: hash  filename
    # We need to handle filenames with spaces/special chars
    
    # Collect all files, compute md5, then process
    
    # Create a temp file to store hash and path
    local tmpfile
    tmpfile=$(mktemp)
    trap "rm -f '$tmpfile'" EXIT
    
    # Find regular files and compute md5
    # Use find with -exec to run md5 -r on each file
    # md5 -r outputs: "hash  filename" (two spaces between hash and filename)
    # We need to parse this carefully
    
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | \
    awk '{
        # md5 -r output format: "hash  filename"
        # The hash is the first field, the rest is the filename
        hash = $1
        # Remove the hash and the two spaces from the beginning
        # The filename starts after the first two spaces
        sub(/^[a-fA-F0-9]+  /, "", $0)
        filename = $0
        # Store hash -> filenames mapping
        if (hash in files) {
            files[hash] = files[hash] "\n" filename
        } else {
            files[hash] = filename
        }
        hashes[hash] = 1
    }
    END {
        for (hash in hashes) {
            n = split(files[hash], arr, "\n")
            if (n >= 2) {
                # Sort the filenames within the group
                # Simple insertion sort
                for (i = 2; i <= n; i++) {
                    key = arr[i]
                    j = i - 1
                    while (j > 0 && arr[j] > key) {
                        arr[j+1] = arr[j]
                        j--
                    }
                    arr[j+1] = key
                }
                # Build the output line
                line = arr[1]
                for (i = 2; i <= n; i++) {
                    line = line " " arr[i]
                }
                print line
            }
        }
    }' | sort
}