function csv_col() {
    local col=$1
    local header
    read header
    IFS=',' read -ra col_indices <<< "$(echo $header | tr ',' ' ' | awk '{print $NF}')"
    for idx in "${col_indices[@]}"; do
        if [[ $idx == $col ]]; then
            while IFS= read -r line; do
                echo "$line"
            done
        fi
    done
}