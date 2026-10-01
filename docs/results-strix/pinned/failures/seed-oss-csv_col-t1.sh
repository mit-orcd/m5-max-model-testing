csv_col() {
    awk -v target="$1" '
        NR == 1 {
            for (i = 1; i <= NF; i++) {
                if ($i == target) {
                    col = i
                    break
                }
            }
            if (!col) exit 1
        }
        NR > 1 { print $col }
    '
}