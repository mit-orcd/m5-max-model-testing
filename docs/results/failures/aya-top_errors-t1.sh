function top_errors {
    local logfile="$1"
    local n="$2"
    local errors=()
    while IFS= read -r line; do
        if [[ $line == "ERROR:"* ]]; then
            local sig="${line##*ERROR:}"
            errors+=("$sig")
        fi
    done < "$logfile"
    local counts=()
    for e in "${errors[@]}"; do
        counts+=($(echo "$e" | tr -cd "#" | wc -c))
    done
    local i=0
    while [ $i -lt ${#errors[@]} ]; do
        local sig="${errors[$i]}"
        local count="${counts[$i]}"
        if [ $count -gt 0 ]; then
            local j=$(echo -e "${sig//#!/\\#}" | tr -d '#')
            printf "%d %s\n" "$count" "$j"
        fi
        i=$((i+1))
    done | sort -nr | head -n "$n"
}