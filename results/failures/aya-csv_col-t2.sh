csv_col() {
    local name=$1
    local header
    read header < /dev/stdin
    local cols=($(echo "$header" | tr ',' '\n'))
    local col_idx=$(awk -v n=$name 'NR==1{for(i=1;i<=NF;i++)if($i==n){print i;exit}}' <<< "${cols[*]}")
    local col_vals=($(awk -v col=$col_idx 'NR!=1{print $col}' <<< "$(cat /dev/stdin)"))
    for val in "${col_vals[@]}"; do
        echo "$val"
    done
}