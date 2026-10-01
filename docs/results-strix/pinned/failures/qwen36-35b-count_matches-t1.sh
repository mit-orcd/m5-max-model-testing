count_matches() {
    local pattern="$1"
    local file="$2"
    
    if [[ ! -f "$file" ]]; then
        echo 0
        return
    fi
    
    local count
    count=$(grep -c -F -x -- "$pattern" "$file" 2>/dev/null)
    
    if [[ -z "$count" ]]; then
        echo 0
    else
        echo "$count"
    fi
}