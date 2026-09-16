```bash
find_dupes() {
    local dir="$1"
    
    if [[ -z "$dir" || ! -d "$dir" ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    # Create a temporary file to store hash:path pairs
    local tmpfile
    tmpfile=$(mktemp)
    trap "rm -f '$tmpfile'" EXIT

    # Find all regular files, compute MD5, and store in format: hash:path
    # Using -exec with sh to handle filenames with spaces/newlines safely
    # macOS md5 -r outputs "MD5 (file) = hash" for single file, or "MD5 (file) = hash" with path
    # Actually md5 -r format is: "MD5 (filename) = hash"
    # We need to parse this carefully.
    # Better approach: iterate files, compute hash, output "hash path"
    
    while IFS= read -r -d '' file; do
        hash=$(md5 -q "$file" 2>/dev/null)
        # md5 -q outputs just the hash if file exists, but on macOS md5 -q might not exist or behave differently.
        # Standard macOS md5 -r output: "MD5 (path) = hash"
        # Let's use md5 -r and parse it.
        # md5 -r "$file" -> "MD5 (path) = hash"
        # We want to output "hash path"
        
        # Re-reading: md5 -r output format on macOS is "MD5 (filename) = hash"
        # So we can do:
        # line=$(md5 -r "$file")
        # hash=$(echo "$line" | awk '{print $NF}')
        # But we need to handle the path which might have spaces.
        
        # Actually, let's use a more robust parsing.
        # md5 -r "$file" output: "MD5 (file) = hash"
        # We can extract hash as last field, and file as everything between parentheses.
        
        read -r _ _ _ _ hash <<< $(md5 -r "$file" 2>/dev/null)
        # That's fragile if path has spaces.
        
        # Better: use sed or awk to parse md5 -r output
        # md5 -r "$file" -> "MD5 (path) = hash"
        # We can do:
        # output=$(md5 -r "$file")
        # hash=$(echo "$output" | sed 's/^MD5 ([^)]*) = //')
        # But we need to output "hash path"
        
        # Let's try a different approach: use md5 without -r, but that requires piping file content.
        # cat "$file" | md5 -> "MD5 (stdin) = hash"
        # That's worse.
        
        # Let's stick with md5 -r and parse carefully.
        # md5 -r "$file" on macOS outputs: "MD5 (path) = hash"
        # We can extract hash as the last word, and path as the part between parentheses.
        
        line=$(md5 -r "$file" 2>/dev/null) || continue
        if [[ -z "$line" ]]; then continue; fi
        
        # Extract hash: last space-separated token
        hash=$(echo "$line" | awk '{print $NF}')
        
        # Extract path: between parentheses
        # Remove "MD5 (" from start, and " = hash" from end
        path=$(echo "$line" | sed 's/^MD5 (\(.*\) = .*/\1/')
        
        # Output hash and path separated by tab (to handle spaces in path)
        printf '%s\t%s\n' "$hash" "$path"
    done < <(find "$dir" -type f -print0) > "$tmpfile"

    # Sort by hash, then by path
    sort -t$'\t' -k1,1 -k2,2 "$tmpfile" | \
    awk -F'\t' '
    {
        hash = $1
        path = $2
        if (hash != prev_hash) {
            if (prev_count >= 2) {
                # Print previous group
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
        if (prev_count >= 2) {
            print