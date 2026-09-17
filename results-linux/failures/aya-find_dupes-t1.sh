function find_dupes () {
    local dir="$1"
    local md5_sum
    local paths=()
    local group=()
    local groups=()

    md5_sum=$(md5 -r "$dir" | awk -F'[ :]' '{print $NF}')
    if [ -z "$md5_sum" ]; then
        return
    fi

    while IFS= read -r path; do
        paths+=("$path")
        if [ -f "$path" ]; then
            group+=("$path")
            if [ ${#group[@]} -ge 2 ]; then
                groups+=("${group[@]}")
                group=()
            fi
        fi
    done < <(find "$dir" -type f -exec md5 -r {} + | awk -F'[ :]' '{print $NF}' | sort | uniq -d | tr -d '\n')

    if [ ${#group[@]} -ge 2 ]; then
        groups+=("${group[@]}")
    fi

    for group in "${groups[@]}"; do
        echo "${group[@]}"
    done
}