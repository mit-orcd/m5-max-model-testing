csv_to_tsv() {
    local file="$1"
    local in_quotes=0
    local field=""
    local char
    local prev_char
    local line=""
    local output=""
    local first_field=1
    
    # Read file character by character
    while IFS= read -r -n1 char; do
        if [[ $in_quotes -eq 1 ]]; then
  # Inside quoted field
            if [[ "$char" == '"' ]]; then
  # Check next character
                read -n1 prev_char
                if [[ "$prev_char" == '"' ]]; then
  # Escaped quote ""
                    field+="$char"
                else
  # End of quoted field
                    in_quotes=0
  # Put back the next character
                    if [[ -n "$prev_char" ]]; then
  # Process the character we read
                        if [[ "$prev_char" == ',' ]]; then
  # Field separator
                            if [[ $first_field -eq 1 ]]; then
                                output="$field"
                                first_field=0
                            else
                                output+="\t$field"
                            fi
                            field=""
                        elif [[ "$prev_char" == '\n' ]] || [[ "$prev_char" == '\r' ]]; then
  # Line ending - complete the record
                                if [[ $first_field -eq 1 ]]; then
                                    output="$field"
                                else
                                    output+="\t$field"
                                fi
                                echo "$output"
                                output=""
                                field=""
                                first_field=1
                            fi
                        else
  # Not a separator, put back
                            field+="$char"
                            field+="$prev_char"
                        fi
                    fi
                fi
            elif [[ "$char" == $'\n' ]]; then
  # Newline in quoted field - convert to \n
                field+='\\n'
            elif [[ "$char" == $'\r' ]]; then
  # CR in quoted field - skip (will be followed by LF)
                continue
            elif [[ "$char" == $'\t' ]]; then
  # Tab in quoted field - convert to \t
                field+='\\t'
            else
                field+="$char"
            fi
        else
  # Outside quoted field
            if [[ "$char" == '"' ]]; then
  # Start of quoted field
                in_quwired=1
                field=""
            elif [[ "$char" == ',' ]]; then
  # Field separator
                if [[ $first_field -eq 1 ]]; then
                    output="$field"
                    first_field=0
                else
                    output+="\t$field"
                fi
                field=""
            elif [[ "$char" == $'\n' ]]; then
  # End of record
                if [[ $first_field -eq 1 ]]; then
                    output="$field"
                else
                    output+="\t$field"
                fi
                echo "$output"
                output=""
s            field=""
                first_field=1
            elif [[ "$char" == $'\r' ]]; then
  # Skip CR (will be followed by LF which we skip too)
                continue
            else
                field+="$char"
            fi
        fi
    done < "$file"
    
  # Handle last line if no trailing newline
    if [[ -n "$field" ]] || [[ $first_field -eq 0 ]]; then
        if [[ $first_field -eq 1 ]]; then
            output="$field"
        else
            output+="\t$field"
        fi
        echo "$output"
    fi
}