csv_col() {
    local -n name="$1"
    local header
    read -r header
    local cols=(${header//,/ })
    local idx=0
    for col in "${cols[@]}"; do
        if [[ "$col" == "$name" ]]; then
            idx=$i
            break
        fi
        ((i++))
    done
    while read -r line; do
        local vals=(${line//,/ })
        [[ ${vals[$idx]} ]] && echo "${vals[idx]}"
    done
}