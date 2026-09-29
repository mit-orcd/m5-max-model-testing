count_matches() {
    local pattern="$1"
    local file="$2"
    
    if [[ ! -f "$file" ]]; then
        echo "0"
        return
    fi
    
    local count=$(grep -c "^$pattern$" "$file" 2>/dev/null)
    echo "${count:-0}"
}