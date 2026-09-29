count_matches() {
    local pattern="$1"
    local file="$2"
    
    if [[ -f "$file" ]]; then
        grep -c "^$pattern$" "$file" 2>/dev/null | grep -c .
    else
        echo 0
    fi
}