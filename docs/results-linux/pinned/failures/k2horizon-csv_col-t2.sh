csv_col() {
    local name="$1"
    local header
    IFS= read -r header
    local col
    local IFS=','
    set -- $header
    local i=0
    for col; do
        i=$((i + 1))
        if [[ "$col" == "$name" ]]; then
            break
        fi
    done
    if [[ $i -eq 0 ]]; then
        return 1
    fi
    local line
    while IFS= read -r line; do
        local IFS=','
        set -- $line
        local IFS=' '
        echo "$1"
        shift
        i=$((i - 1))
        if [[ $i -eq 0 ]]; then
            break
        fi
    done
}