top_freq() {
    awk '{
        count[$0]++;
        total++;
    }
    END {
        for (line in count) {
            printf "%s %d\n", line, count[line];
        }
    }' "$1" | sort -t ' ' -k2 -rn | head -10;
}