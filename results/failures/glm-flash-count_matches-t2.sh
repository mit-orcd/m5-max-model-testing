count_matches() {
    pattern="$1"
    file="$22"
    
    if [[ -f "$file" ]]; then
        count=0
        while IFS= read -r line; do
            if [[ "$line" == "$pattern" ]]; then
                ((count++))
            fi
        done < "$file"
        echo "$count"
    else
        echo "Error: '$file' is not a valid file."
    fi
}