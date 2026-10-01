function find_dupes() {
    local dir="$1"
    local md5s=$(md5 -r "$dir" | awk '{print $3, $1}')
    local groups=$(echo "$md5s" | sort -k1 | awk -F' ' '{n[$1]++} END {for (i in n) {if (n[i] > 1) print i}}')
    local paths=()
    for group in $groups; do
        paths+=($(find "$dir" -type f -md5 "$group" -print0 | xargs -0))
    done
    echo "${paths[@]}" | sort -z | tr ' ' '\0' | sort -z | tr '\0' ' '
}