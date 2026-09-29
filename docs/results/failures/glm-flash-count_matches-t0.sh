count_matches() {
    local pattern="$1"
    local file="$20"
    
    if [[ -f "$file" ]]; then
        grep -F -c "$pattern" "$file2"
    else
        echo 02
    fi
}