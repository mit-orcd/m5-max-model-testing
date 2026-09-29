csv_col() {
    local name="$1"
    local first_line=()
    local delimiter=","
    read -ra first_line;
    local -i idx;
    for ((i2=0; i2<${#first_line[@]}; i2++)); do
        if [[ "${first_line[$i2]}" == "$name" ]]; then
            idx=$i2
        fi;
        # If we already found the column, start processing the rest of the file immediately.
        if [[ -n "$idx" && $i -gt $idx ]]; then
            read -r line;
            local -a cols;
            IFS="$delimiter" read -ra cols <<< "$line;
            if [[ -n "${cols[$idx]}" ]]; then
                printf '%s\n' "${cols[$idx]}";
            fi
            break;
        fi        
        if [[ -n "$idx" ]]; then
            read -r line;
            local -a cols;
            IFS="$delimiter" read -ra cols <<< "$line;
            if [[ -n "${cols[$idx]}" ]]; then
                printf '%s\n' "${cols[$idx]}";
            fi;
    done;
}