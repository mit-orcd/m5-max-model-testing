csv_to_tsv() {
    local IFS=,
    local -a fields
    while IFS= read -r line || [ -n "$line" ]; do
        # Remove carriage returns
        line="${line//$(printf '\r')/}"
        
        # Handle quoted fields
        while [[ "$line" == \"*\" ]]; do
            line="${line:1:${#line}-2}"
            # Replace double quotes with single quote
            line="${line//\"/\' }"
            # Escape single quotes (not standard in RFC, but needed for shell parsing)
            line="${line//\'/\'\\\'\'}"
        done
        
        # Split line into fields
        read -ra fields <<< "$line"
        
        # Process each field
        for i in "${!fields[@]}"; do
            f="${fields[$i]}"
            # Remove surrounding quotes if present
            if [[ "$f" == \"*\" ]]; then
                f="${f:1:${#f}-2}"
            fi
            # Escape tabs and newlines
            f="${f//\\/\\\\}"
            f="${f//\t/\\t}"
            f="${f//$'\n'/\\n}"
            f="${f//$'\r'/\\r}"
            # Restore escaped double quotes
            f="${f//\'/\"}"
            fields[$i]="$f"
        done
        
        # Join fields with tabs
        printf '%s\t' "${fields[@]}"
        printf '\n'
    done
}