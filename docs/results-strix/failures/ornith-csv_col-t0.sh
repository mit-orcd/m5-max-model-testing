csv_col() {
    local name="$1"
    local header line
    IFS=',' read -r header <&0
    local cols=($header)
    local idx=-1 i
    for i in "${!cols[@]}"; do
        if [[ "${cols[$i]}" == "$name" ]]; then
            idx=$i
            break
        fi
    done
    if [[ $idx -eq -1 ]]; then
        return 1
    fi
    while IFS=',' read -r line; do
        local fields=($line)
        echo "${fields[$idx]}"
    done
}