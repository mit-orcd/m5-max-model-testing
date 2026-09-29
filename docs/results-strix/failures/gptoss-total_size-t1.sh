#!/usr/bin/env bash

total_size() {
    local dir="$1"
    local sum=0

    # Return 0 if directory doesn't exist or isn't a directory
    if [ ! -d "$dir" ]; then
        printf '%s\n' 0
        return
    fi

    # Use null-separated names to safely handle all filenames
    while IFS= read -r -d '' file; do
        # Get file size in bytes; stat -f %z works on macOS BSD
        local sz
        sz=$(stat -f %z "$file")
        sum=$((sum + sz))
    done < <(find "$dir" -type f -print0)

    printf '%s\n' "$sum"
}