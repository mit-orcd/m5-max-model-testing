function find_dupes() {
    local dir="$1"
    local md5s=$(md5 -r "$dir" | awk '{print $4, $1}')
    local groups=$(echo "$md5s" | sort -k1 | awk -F' ' '{n[$1]++} END {for (i in n) {if (n[i] > 1) print i}}')
    local paths=()
    for group in $groups; do
        paths+=($(find "$dir" -type f -exec md5 -r {} + | awk -v md5="$group" '($1 == md5) {print $2}'))
    done
    local sorted_paths=($(sort -V <<< "${paths[*]}"))
    local result=()
    for path in "${sorted_paths[@]}"; do
        result+=("$path")
        if [[ ${#result[@]} -gt 1 ]]; then
            echo "${result[@]}"
            result=()
        fi
    done
}