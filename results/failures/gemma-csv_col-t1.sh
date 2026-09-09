csv_col() {
    local col_name="$1"
    local awk_script='
        BEGIN { FS = ","; OFS = "\n" }
        NR == 1 {
            for (i = 1; i <= NF; i++) {
                if ($i == col_name) {
                    target = i
                    break
                }
            }
            exit
        }
        NR > 1 && target { print $target }
    '
    awk -v col_name="$col_name" "$awk_script"
}