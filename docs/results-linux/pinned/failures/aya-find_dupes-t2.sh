function find_dupes() {
    local dir="$1"
    local md5s=$(md5 -r "$dir" | awk '{print $1, $2}')
    local groups=$(echo "$md5s" | sort -k2 | uniq -w32 -c -i | awk '!seen[$2]++ {groups[$1]=$2} END {for (i in groups) print groups[i], i}')
    local paths=$(echo "$groups" | awk '{print $2}' | xargs -I{} find "$dir" -type f -md5 {} | sort -k3 | awk '!seen[$0]++ {paths[$1]=$0} END {for (i in paths) print paths[i]}')
    echo "$paths"
}