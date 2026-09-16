csv_to_tsv() {
    local file="$1"
    if [[ ! -f "$file" ]]; then
        return 1
    fi

    # Read the entire file, handling both LF and CRLF line endings
    # We'll process character by character to properly handle quoted fields with embedded newlines
    local input
    input=$(<"$file")

    # Normalize CRLF to LF
    input="${input//$'\r\n'/$'\n'}"
    # Also handle lone CR as line ending if present (less common but robust)
    # Actually, RFC 4180 says CRLF, but let's just strip all CRs to be safe as per "CR must not survive"
    input="${input//$'\r'/}"

    local len=${#input}
    local i=0
    local record=""
    local field=""
    local in_quotes=0
    local first_field=1
    local has_output=0

    # Function to output a completed record
    output_record() {
        if [[ -n "$record" || $has_output -eq 1 ]]; then
            printf '%s\n' "$record"
            has_output=1
        fi
        record=""
        first_field=1
    }

    while [[ $i -lt $len ]]; do
        local char="${input:$i:1}"

        if [[ $in_quotes -eq 1 ]]; then
            if [[ "$char" == '"' ]]; then
                # Check next character
                local next_char="${input:$((i+1)):1}"
                if [[ "$next_char" == '"' ]]; then
                    # Escaped double quote
                    field+='"'
                    i=$((i+2))
                    continue
                else
                    # End of quoted field
                    in_quotes=0
                    i=$((i+1))
                    continue
                fi
            else
                # Inside quoted field: check for special characters
                if [[ "$char" == $'\n' ]]; then
                    field+="\\n"
                elif [[ "$char" == $'\t' ]]; then
                    field+="\\t"
                else
                    field+="$char"
                fi
                i=$((i+1))
                continue
            fi
        else
            # Not in quotes
            if [[ -z "$field" && -z "$record" && "$char" == '"' ]]; then
                # Start of a quoted field (only if field is empty and record is empty or we just started a new field)
                in_quotes=1
                i=$((i+1))
                continue
            elif [[ "$char" == ',' ]]; then
                # End of field
                if [[ $first_field -eq 1 ]]; then
                    record="$field"
                    first_field=0
                else
                    record+=$'\t'"$field"
                fi
                field=""
                i=$((i+1))
                continue
            elif [[ "$char" == $'\n' ]]; then
                # End of record
                if [[ $first_field -eq 1 ]]; then
                    record="$field"
                else
                    record+=$'\t'"$field"
                fi
                output_record
                field=""
                i=$((i+1))
                continue
            else
                field+="$char"
                i=$((i+1))
                continue
            fi
        fi
    done

    # Handle the last field/record if there's no trailing newline
    if [[ -n "$field" || $has_output -eq 1 ]]; then
        if [[ $first_field -eq 1 ]]; then
            record="$field"
        else
            record+=$'\t'"$field"
        fi
        output_record
    fi
}