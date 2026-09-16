function top_errors() {
    local file="$1"
    local n="$2"
    local err_counts=()
    local err_sigs=()

    if [[ ! -f "$file" ]]; then
        echo "File not found: $file"
        return 1
    fi

    while IFS= read -r line; do
        if [[ $line == ERROR:* ]]; then
            local sig="${line##*ERROR:}"
            sig="${sig//[0-9]/#}"
            err_counts+=("$sig")
        fi
    done < "$file"

    for sig in "${err_counts[@]}"; do
        err_sigs+=("$sig")
    done

    local sorted_sigs=($(sort -t' ' -k1,1nr -k2,2a <<<"${err_sigs[*]}"))

    if [[ ${#sorted_sigs[@]} -eq 0 ]]; then
        return
    fi

    head -n "$n" <<<"${sorted_sigs[@]}"
}