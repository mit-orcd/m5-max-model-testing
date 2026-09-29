csv_to_tsv() {
    local file="$1"
    local line
    local record=""
    local field=""
    local in_quotes=0
    local prev_char=""
    local char
    local i
    local len
    local out=""
    local field_count=0

    # Read the entire file, handling CRLF and LF
    # We process character by character to handle quoted fields with newlines

    # Read file content, stripping CR characters
    local content
    content=$(cat "$file" | tr -d '\r')

    # Process character by character
    len=${#content}
    i=0

    while [ $i -lt $len ]; do
        char="${content:$i:1}"

        if [ $in_quotes -eq 1 ]; then
            if [ "$char" = '"' ]; then
                # Check if next char is also a quote (escaped quote)
                if [ $((i + 1)) -lt $len ] && [ "${content:$((i + 1)):1}" = '"' ]; then
                    field="${field}\""
                    i=$((i + 2))
                    continue
                else
                    # End of quoted field
                    in_quotes=0
                    i=$((i + 1))
                    continue
                fi
            elif [ "$char" = $'\n' ]; then
                # Newline inside quoted field -> backslash n
                field="${field}\\n"
                i=$((i + 1))
                continue
            elif [ "$char" = $'\t' ]; then
                # Tab inside quoted field -> backslash t
                field="${field}\\t"
                i=$((i + 1))
                continue
            else
                field="${field}${char}"
                i=$((i + 1))
                continue
            fi
        else
            # Not in quotes
            if [ "$char" = '"' ]; then
                # Start of quoted field
                in_quotes=1
                i=$((i + 1))
                continue
            elif [ "$char" = ',' ]; then
                # End of field
                if [ $field_count -gt 0 ]; then
                    out="${out}\t"
                fi
                out="${out}${field}"
                field=""
                field_count=$((field_count + 1))
                i=$((i + 1))
                continue
            elif [ "$char" = $'\n' ]; then
                # End of record
                if [ $field_count -gt 0 ]; then
                    out="${out}\t"
                fi
                out="${out}${field}"
                printf '%s\n' "$out"
                out=""
                field=""
                field_count=0
                i=$((i + 1))
                continue
            else
                field="${field}${char}"
                i=$((i + 1))
                continue
            fi
        fi
    done

    # Handle last record without trailing newline
    if [ -n "$field" ] || [ $field_count -gt 0 ]; then
        if [ $field_count -gt 0 ]; then
            out="${out}\t"
        fi
        out="${out}${field}"
        printf '%s\n' "$out"
    fi
}