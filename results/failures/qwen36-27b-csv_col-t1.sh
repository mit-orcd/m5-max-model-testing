csv_col() {
    local name="$1"
    local col_idx
    col_idx=$(head -n 1 | tr ',' '\n' | grep -n "^${name}$" | head -n 1 | cut -d: -f1)
    if [[ -z "$col_idx" ]]; then
        return 1
    fi
    tail -n +2 | cut -d',' -f"$col_idx"
}