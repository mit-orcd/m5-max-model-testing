find_dupes() {
    local dir="${1:-.}"
    local tmpfile
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT

    # Find all regular files, compute md5 for each
    # Using find and md5 -r (macOS/BSD compatible)
    # md5 -r outputs: "hash  filename" or "hash *filename" depending on version
    # On macOS, md5 -r outputs: "MD5 (filename) = hash" is NOT correct; actually:
    # macOS md5 -r outputs: "hash  filename" (two spaces) or "hash *filename"
    # Let's use a more portable approach: md5 file | awk to extract hash

    # Actually, let's use: for each file, run md5 -r which gives "hash  filename"
    # But to be safe across systems, let's parse carefully.
    # On macOS: md5 -r file -> "hash  file" (two spaces between hash and filename)
    # On some BSDs: similar. Let's use this format.

    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | \
    awk '{
        # md5 -r output: hash  filename (two spaces)
        # The hash is first 32 chars, then two spaces, then filename
        hash = substr($0, 1, 32)
        # Find the filename: skip "hash  " (34 chars total: 32 hash + 2 spaces)
        # But if filename starts with space, this breaks. Better to split on first two spaces.
        idx = index($0, "  ")
        if (idx > 0) {
            filename = substr($0, idx + 2)
            print hash "\t" filename
        }
    }' | \
    sort -t$'\t' -k1,1 -k2,2 > "$tmpfile"

    # Now process: group by hash, collect filenames, output groups with 2+ files
    awk -F'\t' '
    {
        hash = $1
        filename = $2
        if (hash != prev_hash && prev_hash != "") {
            if (count >= 2) {
                # Sort the filenames (they should already be sorted due to input sort)
                # But we collected them in order, so just print
                printf "%s", sorted_lines[1]
                for (i = 2; i <= count; i++) {
                    printf " %s", sorted_lines[i]
                }
                printf "\n"
            }
            count = 0
            delete sorted_lines
        }
        prev_hash = hash
        count++
        sorted_lines[count] = filename
    }
    END {
        if (count >= 2) {
            printf "%s", sorted_lines[1]
            for (i = 2; i <= count; i++) {
                printf " %s", sorted_lines[i]
            }
            printf "\n"
        }
    }
    ' "$tmpfile"
}