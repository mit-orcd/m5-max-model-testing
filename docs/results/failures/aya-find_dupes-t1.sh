function find_dupes () {
    dir="$1"
    find "$dir" -type f -exec md5 -r {} + | sort -k1,1 -o - | md5 -r -c - | awk -F'[ :]' '$NF>1' | sort -k1,1
}