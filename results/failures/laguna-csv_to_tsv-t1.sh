csv_to_tsv() {
    local file="$1"
    local line buffer field quoted value
    local IFS=','
    
    while IFS= read -r line || [[ -n "$line" ]]; do
        # Normalize line endings
        line="${line%$'\r'}"
        buffer=""
        
        while [[ -n "$line" ]]; do
            # Check if field is quoted
            if [[ "$line" == \"* ]]; then
                quoted=1
                line="${line:1}"  # Remove leading quote
                field=""
                
                while [[ -n "$line" ]]; do
                    if [[ "$line" == \"* ]]; then
                        # Check for escaped quote
                        if [[ "${line:1:1}" == "\"" ]]; then
                            field+="\"\"
                            line="${line:2}"
                        else
                            # End of quoted field
                            line="${line:1}"
                            break
                        fi
                    else
                        field+="${line:0:1}"
                        line="${line:1}"
                    fi
                done
                
                # Handle remaining line after quoted field
                if [[ -n "$line" && "$line" != ","* ]]; then
                    # Shouldn't happen in valid CSV, but handle gracefully
                    field+="$line"
                    line=""
                fi
                
                # Remove surrounding quotes and process escapes
                value="${field#\"}"
                value="${value%\"}"
                value="${value//\\n/\\n}"
                value="${value//\\t/\\t}"
                value="${value//\"\"/\"}"
            else
                # Unquoted field - take everything up to comma or end
                if [[ "$line" == *","* ]]; then
                    field="${line%%,*}"
                    line="${line#*,}"
                else
                    field="$line"
                    line=""
                fi
                value="$field"
            fi
            
            if [[ -n "$buffer" ]]; then
                buffer+=$'\t'
            fi
            buffer+="$value"
        done
        
        echo "$buffer"
    done < "$file"
}