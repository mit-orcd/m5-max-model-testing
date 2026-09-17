function csv_col() {
    local col=$1
    local header=$(head -n 1)
    local col_idx=$(awk 'FNR==1{print FNR}' <(echo "$header") | awk '{for(i=1;i<=NF;i++) print i}')
    local col_idx=$(echo "$col_idx" | tr -d '[:space:]')

    sed -n -e "/^$header$/ {n;}" -e "1,/^$header$/d" -e "s/\s*$col\s*//" |
    awk -v col_idx="$col_idx" -F, '{$1=substr($0,col_idx)}1' |
    sort -t, -k1,1n |
    awk -F, '{$1=$1}1' |
    awk -v col="$col" '{$1=substr($1,1,length(col))}1'
}