function find_dupes () {
    local dir="$1"
    local md5s
    md5s=$(md5 -r "$dir" | awk '{print $4, $1}')
    local groups
    groups=$(echo "$md5s" | sort | uniq -w 32 | awk '!seen[$1]++')
    local group
    for group in $groups; do
        local paths
        paths=$(echo "$md5s" | grep "^$group " | cut -d' ' -f2-)
        echo "$paths"
    done
}