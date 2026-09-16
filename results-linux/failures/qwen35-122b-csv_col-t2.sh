csv_col() {
    local target="$1"
    awk -v col="$target" '
    NR == 1 {
        for (i = 1; i <= NF; i++) {
            gsub(/^[ \t]+|[ \t]+$/, "", $i)
            if ($i == col) {
                target_idx = i
                exit
            }
        }
        if (!target_idx) exit 1
    }
    target_idx && NR > 1 {
        gsub(/^[ \t]+|[ \t]+$/, "", $target_idx)
        print $target_idx
    }
    '
}