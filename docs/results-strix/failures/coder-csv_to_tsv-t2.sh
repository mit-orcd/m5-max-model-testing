csv_to_tsv() {
    local file="$1"
    local IFS=',' read -r -a fields
    local line
    local in_quotes=false
    local field=""
    local output=""
    
    while IFS= read -r line || [[ -n "$line" ]]; do
        # Remove trailing CR if present
        line="${line%$'\r'}"
        
        # Process each character in the line
        local i=0
        local len=${#line}
        field=""
        in_quotes=false
        
        while [ $i -lt $len ]; do
            local char="${line:$i:1}"
            local next_char=""
            [ $((i+1)) -lt $len ] && next_char="${line:$((i+1)):1}"
            
            if [ "$char" = '"' ]; then
                if [ "$in_quotes" = true ]; then
                    if [ "$next_char" = '"' ]; then
                        # Escaped quote
                        field="$field\""
                        i=$((i+1))
                    else
                        # End of quoted field
                        in_quotes=false
                    fi
                else
                    # Start of quoted field
                    in_quotes=true
                fi
            elif [ "$char" = ',' ] && [ "$in_quotes" = false ]; then
                # End of field
                output="$output$field"$'\t'
                field=""
            elif [ "$char" = '\' ] && [ "$next_char" = 'n' ] && [ "$in_quotes" = true ]; then
                # Convert \n to actual newline
                field="$field"$'\n'
                i=$((i+1))
            elif [ "$char" = '\' ] && [ "$next_char" = 't' ] && [ "$in_quotes" = true ]; then
                # Convert \t to actual tab
                field="$field"$'\t'
                i=$((i+1))
            else
                field="$field$char"
            fi
            i=$((i+1))
        done
        
        # Handle last field
        output="$output$field"$'\n'
    done < "$file"
    
    # Remove trailing newline and add one
    output="${output%$'\n'}"$'\n'
    echo -n "$output"
}