find_dupes() {
    local dir="$1"
    if [[ -z "$dir" ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    # Find all regular files recursively, compute MD5, and group by hash
    # macOS: md5 -r outputs "hash  filename"
    local tmpfile
    tmpfile=$(mktemp)

    # Find regular files, compute md5 -r for each
    # Using find with -type f and executing md5 -r on each file
    # Collect results in a temp file: "hash  path"
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        # Use md5 -r which outputs: <hash>  <filename>
        # Note: md5 -r on macOS outputs the hash followed by two spaces and the filename
        md5 -r "$file"
    done > "$tmpfile"

    # Sort by hash to group duplicates, then process
    # We need to:
    # 1. Group files by hash
    # 2. Only keep groups with 2+ files
    # 3. Sort paths within each group
    # 4. Sort groups by first path
    # 5. Print space-separated paths per line

    # Use awk to process the sorted-by-hash file
    sort "$tmpfile" | awk '{
        hash = $1
        # The filename is everything after the first field and the two-space separator
        # md5 -r output format: "hash  filename"
        # So we reconstruct the filename from $2 onwards
        fname = ""
        for (i = 2; i <= NF; i++) {
            if (i == 2) fname = $i
            else fname = fname " " $i
        }
        # Store files per hash
        if (hash in files) {
            files[hash] = files[hash] "\n" fname
        } else {
            files[hash] = fname
            hashes[++nh] = hash
        }
        count[hash]++
    }
    END {
        # For each hash with count >= 2, sort the files and output
        ngroups = 0
        for (h in files) {
            if (count[h] >= 2) {
                ngroups++
                group_hashes[ngroups] = h
            }
        }
        
        # For each group, sort the file paths
        # We will store the sorted lines in an array, then sort by first path
        for (i = 1; i <= ngroups; i++) {
            h = group_hashes[i]
            # Split files by newline
            n = split(files[h], farr, "\n")
            
            # Sort the file paths (simple insertion sort)
            for (j = 2; j <= n; j++) {
                key = farr[j]
                k = j - 1
                while (k >= 1 && farr[k] > key) {
                    farr[k + 1] = farr[k]
                    k--
                }
                farr[k + 1] = key
            }
            
            # Build space-separated string
            line = farr[1]
            for (j = 2; j <= n; j++) {
                line = line " " farr[j]
            }
            sorted_lines[i] = line
            first_paths[i] = farr[1]
        }
        
        # Sort groups by first path (insertion sort on indices)
        for (i = 2; i <= ngroups; i++) {
            key_line = sorted_lines[i]
            key_first = first_paths[i]
            k = i - 1
            while (k >= 1 && first_paths[k] > key_first) {
                sorted_lines[k + 1] = sorted_lines[k]
                first_paths[k + 1] = first_paths[k]
                k--
            }
            sorted_lines[k + 1] = key_line
            first_paths[k + 1] = key_first
        }
        
        # Print all groups
        for (i = 1; i <= ngroups; i++) {
            print sorted_lines[i]
        }
    }'

    rm -f "$tmpfile"
}