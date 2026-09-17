top_freq() {
    local file="$1"
    local counts=( $(sort -r -k1n <<<$(awk -F' ' '{print $2, $1}' <"$file" | sort -k1 | uniq -c | tr -d '[]' | awk -F' ' '{print $2, $1}') )
    local top=10
    local i
    for i in "${counts[@]}"; do
        echo "${counts[${#counts[@]}-$top]} ${i#* }"
        top=$((top-1))
    done
}