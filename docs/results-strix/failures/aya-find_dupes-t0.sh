function find_dupes() {
    local dir="$1"
    local md5s=$(md5 -r "$dir" | awk '{print $1, $2}')
    local groups=$(echo "$md5s" | sort -k2 | uniq -w32 -c -i | awk '{$1=$2=$3="";print $0}')
    local paths=$(echo "$groups" | awk '{print $1}')
    local paths_sorted=$(echo "$paths" | sort)
    local groups_sorted=$(echo "$groups" | sort -k1)
    local output=()
    for path in $paths_sorted; do
        if [[ $path == *$dir* ]]; then
            output+=("$path")
        fi
    done
    for group in $groups_sorted; do
        local count=$(echo "$group" | awk '{print $1}')
        local md5=$(echo "$group" | awk '{print $2}')
        local paths_group=$(echo "$group" | awk '{print $3}')
        if [[ $count -gt 1 ]]; then
            output+=("$md5 $paths_group")
        fi
    done
    echo "${output[@]}"
}