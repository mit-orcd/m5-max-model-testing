function top_errors() {
    local file="$1"
    local n="$2"
    local count
    local signature
    local sig_count=()
    local -a sig_order=()
    local -A sig_counts

    while IFS= read -r line; do
        if [[ $line == ERROR:* ]]; then
            signature=${line##*ERROR:}
            signature=${signature//[0-9]/#}
            sig_counts[$signature]=$((sig_counts[$signature] + 1 || 1))
        fi
    done < "$file"

    for signature in "${!sig_counts[@]}"; do
        count=${sig_counts[$signature]}
        sig_count+=("$count $signature")
    done

    sig_order=($(sort -rn <<<"${sig_count[*]}"))

    echo "${sig_order[0..$n - 1]}"
}