function sum_stdin() {
    local sum=0
    while IFS= read -r num; do
        [[ -z "$num" ]] && continue
        sum+=$num
    done
    echo "$sum"
}