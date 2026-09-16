count_matches() {
    local pattern="$1"
    local file="$2"
    local count

    # If arguments are missing, return 0
    if [[ -z "$pattern" || -z "$file" ]]; then
        echo 0
        return
    fi

    count=$(grep -F -c -- "$pattern" -- "$file" 2>/dev/null)
    if [[ $? -ne 0 ]]; then
        count=0
    fi

    echo "$count"
}