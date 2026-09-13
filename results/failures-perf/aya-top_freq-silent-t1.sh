top_freq() {
    local file="$1"
    local counts=()
    local -A count_by_value

    while IFS= read -r value; do
        count_by_value["$value"]=$((count_by_value["$value"] + 1 || 1))
    done < "$file"

    for value in "${!count_by_value[@]}"; do
        counts+=("$((count_by_value["$value"]))) $value")
    done

    sort_num_asc <<< "${counts[@]}" | head -n 10
}

sort_num_asc() {
    local num_field=$1
    local num_sep='"'
    local num_end='"'
    local num_re="^[^$num_sep]*$num_sep\
([-+]?[0-9]+(\.[0-9]+)?|inf|Inf|Infinity)\
[^$num_end]*$num_end"

    while read -r line; do
        if [[ $line =~ $num_re ]]; then
            local num=${BASH_REMATCH[1]}
            echo "${line:$(( ${#BASH_REMATCH[0]}-1 ))}"
        else
            echo "$line"
        fi
    done
}