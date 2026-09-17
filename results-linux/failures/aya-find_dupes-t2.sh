function find_dupes() {
    local dir="$1"
    local md5s
    md5s=$(md5 -r "$dir" | awk '{print $2, $1}')
    local groups=$(echo "$md5s" | sort -k2 | awk '!seen[$2]++')
    local group
    for group in $groups; do
        local paths=$(echo "$md5s" | grep "$group" | cut -d' ' -f1 | sort)
        echo "$paths"
    done
}