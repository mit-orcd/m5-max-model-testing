find_dupes() {
    local dir="${1:?Usage: find_dupes <dir>}"
    
    # Find all regular files, compute MD5, sort by MD5 then path
    # Use find to get files, xargs to run md5, then process
    find "$dir" -type f -print0 | \
        xargs -0 -n100 md5 -r 2>/dev/null | \
        awk '{
            # md5 -r output format: <hash>  <filename>
            # There are two spaces between hash and filename
            hash = $1
            # Reconstruct filename (may contain spaces)
            # Remove the leading two spaces from $0 after hash
            sub(/^[^ ]+  /, "", $0)
            filename = $0
            print hash "\t" filename
        }' | \
        sort -t$'\t' -k1,1 -k2,2 | \
        awk -F'\t' '
        {
            hash = $1
            path = $2
            if (hash == prev_hash) {
                group[group_count] = path
            } else {
                if (group_count >= 2) {
                    # Sort and print the previous group
                    # group[0] to group[group_count-1]
                    # We need to sort the paths in the group
                    n = asort(group, sorted_group)
                    if (n >= 2) {
                        line = sorted_group[1]
                        for (i = 2; i <= n; i++) {
                            line = line " " sorted_group[i]
                        }
                        # Store for final sorting by first path
                        results[result_count++] = line
                    }
                }
                prev_hash = hash
                group_count = 0
                group[group_count++] = path
            }
        }
        END {
            if (group_count >= 2) {
                n = asort(group, sorted_group)
                if (n >= 2) {
                    line = sorted_group[1]
                    for (i = 2; i <= n; i++) {
                        line = line " " sorted_group[i]
                    }
                    results[result_count++] = line
                }
            }
            # Sort results by first path (which is already the first element)
            # Since groups are sorted by hash, and within each group paths are sorted,
            # we need to sort the output lines by the first path
            n = asort(results, sorted_results)
            for (i = 1; i <= n; i++) {
                print sorted_results[i]
            }
        }'
}