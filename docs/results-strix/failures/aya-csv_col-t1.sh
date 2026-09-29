csv_col() {
    local name=$1
    local header=$(head -n 1 <(sort -t$'\t' -k$((2+$(awk -F'\t' 'NR==1{print NF}' <<< "$(<stdin)"))) -V -k1 <<< "$(<stdin)"))
    local -a cols=($(awk -F'\t' 'NR==1{print $NF}' <<< "$header"))
    local idx=$(awk -v col="$name" 'NR==1{for(i=1;i<=NF;i++)if($i==col)print i;exit}' <<< "$header")
    local -a values=($(awk -F'\t' -v idx="$idx" 'NR>1{print $idx}' <<< "$(<stdin)"))
    printf '%s\n' "${values[@]}"
}