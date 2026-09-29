top_freq() {
    local file="$1"
    local count
    local value
    local counts=( $(sort -rn -k1,1 -o /dev/null <<< "$(cat "$file" | tr -d '\r' | awk -F' ' '{print $2, $1}' | sort | uniq -c | awk '{print $2, $1}')") )
    local i=0
    for value in "${counts[@]}"; do
        count=${counts[$i]}
        echo "$count $value"
        i=$((i+1))
        if [ $i -gt 9 ]; then
            break
        fi
    done
}