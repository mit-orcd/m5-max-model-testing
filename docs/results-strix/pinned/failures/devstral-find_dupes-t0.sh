find_dupes() {
    dir="$1"
    find "$dir" -type f -exec md5 -r {} + | sort -k1,1 -k2,2 | uniq -w32 -d -f1 | awk '{print $2}' | sort -k1,1
}