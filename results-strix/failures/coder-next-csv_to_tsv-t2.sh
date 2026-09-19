csv_to_tsv() {
    local file="$1"
    local state="start"
    local field=""
    local c
    local prev=""
    local i=0
    local len
    local char
    local output=""
    
    # Read entire file, handling CRLF/LF
    local content
    content=$(cat "$file" | tr -d '\r')
    
    len=${#content}
    
    while [ $i -lt $len ] || [ "$state" = "quoted" ]; do
        if [ $i -ge $len ]; then
            c=""
        else
            c="${content:$i:1}"
        fi
        
        case "$state" in
            "start")
                if [ "$c" = '"' ]; then
                    state="quoted"
                elif [ "$c" = "" ]; then
                    # End of file, output current field
                    output="${output}${field}"
                    break
                else
                    field="${field}${c}"
                    state="unquoted"
                fi
                ;;
            "unquoted")
                if [ "$c" = "" ]; then
                    output="${output}${field}"
                    break
                elif [ "$c" = "," ]; then
                    output="${output}${field}"$'\t'
                    field=""
                elif [ "$c" = $'\n' ]; then
                    output="${output}${field}"$'\n'
                    field=""
                    state="start"
                else
                    field="${field}${c}"
                fi
                ;;
            "quoted")
                if [ "$c" = '"' ]; then
                    # Look ahead for next char
                    if [ $((i+1)) -lt $len ]; then
                        local next_char="${content:$((i+1)):1}"
                        if [ "$next_char" = '"' ]; then
                            # Escaped quote
                            field="${field}\""
                            i=$((i+1))
                        else
                            # End of quoted field
                            state="quoted_end"
                        fi
                    else
                        # End of file - end of quoted field
                        state="quoted_end"
                    fi
                else
                    case "$c" in
                        $'\t')
                            field="${field}\\t"
                            ;;
                        $'\n')
                            field="${field}\\n"
                            ;;
                        *)
                            field="${field}${c}"
                            ;;
                    esac
                fi
                ;;
            "quoted_end")
                if [ "$c" = "" ]; then
                    output="${output}${field}"
                    break
                elif [ "$c" = "," ]; then
                    output="${output}${field}"$'\t'
                    field=""
                    state="start"
                elif [ "$c" = $'\n' ]; then
                    output="${output}${field}"$'\n'
                    field=""
                    state="start"
                elif [ "$c" = ' ' ] || [ "$c" = $'\t' ]; then
                    # Ignore whitespace after closing quote before comma/newline
                    :
                else
                    # Invalid: non-comma/non-newline after closing quote
                    # Treat as part of field per strict RFC, but for simplicity output error field
                    field="${field}${c}"
                    state="unquoted"
                fi
                ;;
        esac
        i=$((i+1))
    done
    
    # Remove trailing tab from last field if present
    output=$(printf '%s' "$output" | sed 's/\t$//')
    printf '%s\n' "$output"
}