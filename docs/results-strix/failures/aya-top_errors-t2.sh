function top_errors {
    local file="$1"
    local n="$2"

    local err_counts=$(grep -o 'ERROR: \[[^\]]*\] .*' "$file" | sed 's/ERROR: \[[^\]]*\] //; s/[0-9]\{1,3\}/#/g' | sort | uniq -c | sort -nr | head -n "$n")

    local count=0
    local sig=""
    while IFS= read -r line; do
        if [[ $line == ERROR* ]]; then
            count=$((count + 1))
            sig=$(echo "$line" | sed 's/ERROR: \[[^\]]*\] //; s/[0-9]\{1,3\}/#/g')
        fi
    done < "$file"

    if [[ $count -gt 0 ]]; then
        echo "$err_counts"
    fi
}