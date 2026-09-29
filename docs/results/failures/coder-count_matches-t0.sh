count_matches() {
    local pattern="$1"
    local file="$2"
    
    if [[ ! -r "$file" ]]; then
        echo "0"
        return
    fi
    
    grep -c "^.*$pattern.*$" "$file" 2>/dev/null | wc -l
}