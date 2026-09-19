csv_to_tsv() {
    local input_file="$1"
    local IFS=','
    local line

    while IFS= read -r line || [[ -n "$line" ]]; do
        # Remove CRLF line endings
        line="${line///}"
        
        # Remove surrounding double quotes
        if [[ "$line" == \"*\" ]]; then
            line="${line:1:${#line}-2}"
        fi
        
        # Replace double quotes inside the field with a single literal quote
        line="${line//\"\"/\"}"
        
        # Replace escaped characters backslash n and backslash t
        # Note: This handles the sequence \n and \t appearing literally in the output
        line="${line//\\n/$(printf '\n')}"
        line="${line//\\t/$(printf '\t')}"
        
        echo "$line"
    done < "$input_file"
}