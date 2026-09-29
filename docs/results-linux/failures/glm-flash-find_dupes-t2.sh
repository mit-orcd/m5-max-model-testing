find_dupes() {
    local dir="$1"
    # Group files by MD5 hash. 
    # -type f: regular files only.
    # -exec md5 -r {} +: calculate MD5 and print to stdout.
    # sort -k2: sort by hash.
    # uniq -D -w32: print only duplicate lines (keeping first occurrence).
    # awk: join lines with the same hash into a single line.
    find "$dir" -type f -exec md5 -r {} + | sort -k2 | uniq -D -w32 | awk '{
        sum = $1
        for (i = 2; i <= NF; i++) {
            sum = sum " " $i
        }
        print sum
    }' | sort | while read -r line; do
        # Split the line by spaces and sort the paths alphabetically.
        # Print the sorted paths separated by a space.
        echo "$line" | tr ' ' '\n' | sort | tr '\n' ' '
        echo
    done
}