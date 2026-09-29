total_size() {
    local dir="${1:?missing argument}"
    local sum=0
    while IFS= read -r -d '' file; do
        local sz
        sz=$(stat -f %z "$file" 2>/dev/null)
        if [[ -n $sz ]]; then
            sum=$((sum + sz))
        fi
    done < <(find "$dir" -type f -print0 2>/dev/null)
    printf '%s\n' "$sum"
}