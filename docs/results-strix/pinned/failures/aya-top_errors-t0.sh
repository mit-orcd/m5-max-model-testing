function top_errors() {
    local file="$1"
    local n="$2"
    local err_re='^ERROR: ([^#\d]*)(\d+)?'
    local err_sig_re='^ERROR: ([^:]*):'
    local sig_replace_re='(\d+)'
    local sig_replace='#'
    local count
    local sig
    local err_lines
    local err_sig
    local err_counts
    local err_sig_counts
    local max_count=0

    if [[ ! -f "$file" ]]; then
        echo "File not found: $file"
        return 1
    fi

    err_lines=$(grep -E '^ERROR:' "$file" | sort -u)
    err_sig_counts=$(echo "$err_lines" | awk -F: '{print $1}' | sort | uniq -c | sort -nr | head -n "$n")
    err_counts=$(echo "$err_lines" | awk -F: '{print $2}' | sort | uniq -c | sort -nr | head -n "$n")

    for sig in $(echo "$err_sig_counts" | awk '{print $2}'); do
        count=$(echo "$err_counts" | awk '$1 ~/"^'$sig'"/ {print $1}')
        if [[ $count -gt $max_count ]]; then
            max_count=$count
        fi
    done

    echo "$err_sig_counts" | awk -v max="$max_count" '
        $1 ~/"^'$sig'"/ {
            if ($1 == max) {
                print $1, $2
            }
        }
    '

    for sig in $(echo "$err_sig_counts" | awk '{print $2}'); do
        count=$(echo "$err_counts" | awk '$1 ~/"^'$sig'"/ {print $1}')
        if [[ $count -lt $max_count ]]; then
            echo "$count $sig"
        fi
    done
}