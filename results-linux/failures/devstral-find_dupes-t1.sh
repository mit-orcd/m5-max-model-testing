find_dupes() {
    dir="$1"
    md5sums=$(find "$dir" -type f -exec md5 -r {} + | sort -k1,1 -k2,2)
    groups=$(awk 'prev != $1 {if (count > 1) print prev; prev=$1; count=0} {count++; files[NR]=$2} END {if (count > 1) print prev}' <<< "$md5sums")
    while IFS= read -r hash; do
        files=$(awk -v h="$hash" '$1 == h {print $2}' <<< "$md5sums")
        sorted_files=$(echo "$files" | tr ' ' '\n' | sort)
        echo "$sorted_files"
    done <<< "$groups"
}