csv_col() {
    awk -F, -v col="$1" 'NR==1 { for (i=1; i<=NF; i++) if ($i==col) c=i; next } { print $c }'
}
