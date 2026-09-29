find_dupes() {
    dir="$1"
    find "$dir" -type f -exec md5 -r {} + | sort -k1,1 -k2,2 | \
    awk 'prev && $1 == prev {print prev_files " " $2; prev_files = prev_files " " $2} {prev = $1; prev_files = $2}' | \
    sort -k1,1 | awk 'NF > 1 {print}'
}