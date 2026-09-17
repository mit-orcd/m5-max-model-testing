function top_errors() {
    local file="$1"
    local n="$2"
    local errors=()
    local count=()
    local sig=()
    local i

    while IFS= read -r line; do
        if [[ $line == ERROR:* ]]; then
            sig+=("$line")
        fi
    done < "$file"

    for i in "${!sig[@]}"; do
        errors+=("${sig[$i]##ERROR:}")
        count+=("$i")
    done

    for i in {1..n}; do
        echo "${count[$i]} ${errors[$i]}"
    done | sort -k1 -nr -k2
}