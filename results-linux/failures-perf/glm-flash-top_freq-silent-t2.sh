top_freq() {
    awk '{
        count[$0]++
        total++
    }
    END {
        # Sort numerically by count descending, then alphabetically ascending
        for (item in count) {
            print count[item] " " item
        } | sort -rn
        # Print only the first 10 lines
    }' "$1" | head -n 10
}