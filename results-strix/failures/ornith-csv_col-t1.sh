csv_col() {
    local name="$1"
    local header line col
    IFS=',' read -ra header <<< "$(head -n1)"
    col=$(printf '%s\n' "${header[@]}" | grep -nx "$name" | cut -d: -f1)
    if [ -z "$col" ]; then
        echo "Column '$name' not found" >&2
        return 1
    fi
    tail -n +2 | while IFS=',' read -ra values; do
        echo "${values[$((col-1))]}"
    done
}