csv_col() {
    awk -v col="$1" '
        NR==1 { for (i=1; i<=NF; i++) if ($i==col) { c=i; break } }
        NR>1  { print $c }
    '
}