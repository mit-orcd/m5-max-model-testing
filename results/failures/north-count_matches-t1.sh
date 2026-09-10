count_matches() {
    local pattern="$1"
    local file="$2"
    if [ -f "$file" ]; then
        grep -F -c "$pattern" "$file" 2>/dev/null || echo 0
    else
        echo 0
    fi
}