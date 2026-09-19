csv_to_tsv() {
    local file="$1"
    local state="out"
    local field=""
    local output=""
    local c prev_c
    local i len

    # Read entire file as bytes, handling CRLF/LF
    local content
    content=$(cat "$file" | tr -d '\r') || return

    len=${#content}
    i=0

    while (( i < len )); do
        c=${content:i:1}

        case "$state" in
            out)
                if [[ "$c" == '"' ]]; then
                    state="quoted"
                else
                    if [[ "$c" == ',' ]]; then
                        output+=$'\t'
                    elif [[ "$c" == $'\n' ]]; then
                        # Remove trailing tab if present, then add newline
                        output=${output%$'\t'}
                        printf '%s\n' "$output"
                        output=""
                    else
                        field+="$c"
                    fi
                fi
                ;;
            quoted)
                if [[ "$c" == '"' ]]; then
                    # Check for escaped quote
                    if (( i + 1 < len )) && [[ "${content:i+1:1}" == '"' ]]; then
                        field+='"'
                        (( i++ ))
                    else
                        state="quoted_end"
                    fi
                else
                    field+="$c"
                fi
                ;;
            quoted_end)
                if [[ "$c" == ',' ]]; then
                    # Process the field
                    if [[ "$field" == *$'\n'* ]]; then
                        field=${field//$'\n'/\\n}
                    fi
                    if [[ "$field" == *$'\t'* ]]; then
                        field=${field//$'\t'/\\t}
                    fi
                    output+="$field"$'\t'
                    field=""
                    state="out"
                elif [[ "$c" == $'\n' ]]; then
                    # Process the field
                    if [[ "$field" == *$'\n'* ]]; then
                        field=${field//$'\n'/\\n}
                    fi
                    if [[ "$field" == *$'\t'* ]]; then
                        field=${field//$'\t'/\\t}
                    fi
                    output+="$field"
                    output=${output%$'\t'}
                    printf '%s\n' "$output"
                    output=""
                    field=""
                    state="out"
                elif [[ "$c" == ' ' ]] || [[ "$c" == $'\t' ]]; then
                    # Ignore trailing whitespace after closing quote
                    :
                else
                    # Invalid state: unquoted content after closing quote
                    # Treat as part of field per strict RFC, but spec says to convert successfully
                    # We'll treat as error in parsing, but for robustness: keep in field
                    field+='"'"$c"
                    state="quoted"
                fi
                ;;
        esac

        (( i++ ))
    done

    # Handle final field/record
    case "$state" in
        quoted)
            # Unfinished quoted field: treat as literal field
            if [[ "$field" == *$'\n'* ]]; then
                field=${field//$'\n'/\\n}
            fi
            if [[ "$field" == *$'\t'* ]]; then
                field=${field//$'\t'/\\t}
            fi
            printf '%s\n' "$field"
            ;;
        out)
            # Final field may exist
            if [[ -n "$field" ]]; then
                printf '%s\n' "$field"
            elif [[ -n "$output" ]]; then
                output=${output%$'\t'}
                printf '%s\n' "$output"
            fi
            ;;
        quoted_end)
            # Process the field
            if [[ "$field" == *$'\n'* ]]; then
                field=${field//$'\n'/\\n}
            fi
            if [[ "$field" == *$'\t'* ]]; then
                field=${field//$'\t'/\\t}
            fi
            printf '%s\n' "$field"
            ;;
    esac
}