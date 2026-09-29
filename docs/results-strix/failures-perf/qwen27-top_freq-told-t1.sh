top_freq() {
    local file="$1"
    sort --parallel="$(nproc)" "$file" | uniq -c | sort -rn -k1,1 | head -n 10
}