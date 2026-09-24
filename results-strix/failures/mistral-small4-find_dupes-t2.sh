#!/bin/bash

find_dupes() {
    local dir="$1"
    local tmpfile=$(mktemp)
    find "$dir" -type f -exec md5 -r {} + | sort -k1,32 | awk '{
        md5=$1
        path=substr($0, index($0,$2))
        if (md5 != prev) {
            if (NR > 1) {
                if (prev_count >= 2) {
                    for (i in prev_paths) print prev_paths[i]
                }
                delete prev_paths
            }
            prev=md5
            count=0
        }
        prev_paths[++count] = path
    } END {
        if (prev_count >= 2) {
            for (i in prev_paths) print prev_paths[i]
        }
    }' | sort
    rm -f "$tmpfile"
}