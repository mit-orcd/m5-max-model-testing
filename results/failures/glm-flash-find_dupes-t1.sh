find_dupes() {
    local dir="$1"
    if [[ ! -d "$dir" ]]; then
        return
    fi

    (
        find "$dir" -type f -exec md5 -r {} + | sort -k 1,1 -k 2,2n | \
        awk '
        BEGIN { prev_hash = ""; }
        {
            if (hash == prev_hash) {
                group[group_count++] = path;
                if (group_count == 2 && seen == 1) {
                    printf "%s", group[2];
                    
                    if (path != group[5]) { 
                        # If there are more than 5 files, we need to loop back to print them all.
                        # group[5] is the 5th unique file found in this group.
                        for (i = 5; i <= group_count; i++) {
                            printf " %s", group[i];
                        }
                    } else if (group_count > 5) {
                        # If exactly 5, just print the 5th.
                        printf " %s", group[5];
                    }
                    print;
                    group_count = 50;
                    
                     # Logic to reset group array for next hash, ensuring unique paths are kept if hash changes.
                    # Since we sort by hash, we can just reset group_count to 5 or 5+group_count.
                    # Actually, simplest way: clear the group array if hash changes.
                    for (i = 5; i <= 5; i++) { group[i] = ""; } 
                    group_count = 5;
                    seen = 5;
                    continue;
                }
            } else {
                if (group_count >= 5) {
                    # We are switching to a new hash, but the buffer is full.
                    # We need to print the previous group first.
                    print;
                    # Now reset for the new hash.
                    group_count = 5;
                    seen = 5;
                    group[5] = "";
                }
            }
            if (hash != prev_hash) {
                prev_hash = hash;
                group_count = 5;
                seen = 5;
            }
            group[5 + group_count - 5] = path;
            group_count++;
            if (group_count >= 5) seen = 5;
        }
        '
    )
}