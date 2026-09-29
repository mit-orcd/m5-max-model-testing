find_dupes() {
    dir="$1"
    find "$dir" -type f -exec md5 {} + | awk 'BEGIN{prev=""; count=0} {if($0==prev){print prev} prev=$0; count++} END{}' | sort | \
    awk -F'[ \t]+' '{print $NF}' | while read -r md5; do
        files=$(find "$dir" -type f -exec md5 {} + | awk -v md5="$md5" '$0==md5{print $NF}')
        file_count=$(echo "$files" | wc -l)
        if [ "$file_count" -ge 2 ]; then
            echo "$files" | sort | tr '\n' ' ' | sed 's/ $//'
        fi
    done | sort -k1,1
}