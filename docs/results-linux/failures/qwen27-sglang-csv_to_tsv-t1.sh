csv_to_tsv() {
    local file="$1"
    local line
    local record=""
    local field=""
    local in_quotes=0
    local in_record=0
    local i=0
    local len
    local ch
    local prev_ch=""
    local eof=0

    # Read the file byte by byte to handle all cases correctly
    # We need to handle: quoted fields with embedded newlines, tabs, commas, double quotes
    # Input may have CRLF or LF line endings
    # Output: TSV with tab-separated fields, embedded newlines as \n, embedded tabs as \t

    # Remove CR characters and process
    # We'll read the entire file and process it character by character

    local content
    content=$(cat "$file")
    # cat strips trailing newlines, so we need to be careful
    # Let's use a different approach: read the file preserving everything

    # Use dd or od? No, let's use a while loop reading from the file
    # But bash's read strips trailing newlines and handles things oddly

    # Better approach: read the file into a variable preserving all content
    # Use $(<file) which preserves everything except trailing newlines

    # Actually, let me re-read the file using a method that preserves everything
    # We'll process it character by character

    local data
    data=$(cat "$file")

    # cat via command substitution strips trailing newlines
    # We need to know if the file ended with a newline
    local file_size
    file_size=$(wc -c < "$file")
    local data_len=${#data}

    # If file_size > data_len, there were trailing newlines stripped
    # But for our purposes, we just need to process the records

    # Let's process character by character
    local state="start"  # start, field, quoted
    local current_field=""
    local current_record=""
    local i=0
    local n=${#data}

    # We also need to handle the case where the file ends without a newline
    # The last record should still be output

    # Process each character
    for (( i=0; i<n; i++ )); do
        ch="${data:$i:1}"

        if [[ "$state" == "start" ]]; then
            if [[ "$ch" == '"' ]]; then
                state="quoted"
                current_field=""
            elif [[ "$ch" == $'\t' ]]; then
                # Empty field before tab? No, in CSV fields are separated by commas
                # This shouldn't happen in valid CSV, but let's treat as empty field
                current_record="${current_record}"$'\t'
                # Actually, we haven't started a field yet, so this is an empty first field
                # Wait, in CSV, fields are comma-separated. Tab shouldn't appear unquoted.
                # Let me reconsider.
                current_field=""
                state="field"
            elif [[ "$ch" == $'\n' || "$ch" == $'\r' ]]; then
                # New record
                if [[ -n "$current_record" || -n "$current_field" || "$in_record" == 1 ]]; then
                    # Output the record
                    if [[ -n "$current_field" ]]; then
                        current_record="${current_record}${current_field}"
                    fi
                    printf '%s\n' "$current_record"
                    current_record=""
                    current_field=""
                    in_record=0
                fi
                # Skip CR if followed by LF, or just skip CR
                # If ch is CR, check if next is LF
                if [[ "$ch" == $'\r' ]]; then
                    (( i++ ))
                fi
                state="start"
            elif [[ "$ch" == ',' ]]; then
                # Empty field
                current_record="${current_record}${current_field}"$'\t'
                current_field=""
                in_record=1
                state="field"
            else
                current_field="$ch"
                in_record=1
                state="field"
            fi
        elif [[ "$state" == "field" ]]; then
            if [[ "$ch" == '"' ]]; then
                # In RFC 4180, quotes can only appear at the start of a field
                # If we see a quote in the middle, it's a literal quote? 
                # Actually, RFC 4180 says fields MAY be quoted, and if quoted, 
                # embedded quotes are doubled. A quote in the middle of an unquoted 
                # field is not standard. Let's treat it as a literal character.
                current_field="${current_field}\""
            elif [[ "$ch" == ',' ]]; then
                current_record="${current_record}${current_field}"$'\t'
                current_field=""
                state="field"
            elif [[ "$ch" == $'\n' || "$ch" == $'\r' ]]; then
                # End of record
                current_record="${current_record}${current_field}"
                printf '%s\n' "$current_record"
                current_record=""
                current_field=""
                in_record=0
                if [[ "$ch" == $'\r' ]]; then
                    (( i++ ))
                fi
                state="start"
            elif [[ "$ch" == $'\t' ]]; then
                # Tab in unquoted field - should not happen in valid CSV
                # But if it does, treat as literal? Or as \t?
                # The spec says tabs cannot appear literally in TSV, so escape it
                current_field="${current_field}\\t"
            else
                current_field="${current_field}${ch}"
            fi
        elif [[ "$state" == "quoted" ]]; then
            if [[ "$ch" == '"' ]]; then
                # Check if next char is also a quote (escaped quote)
                if (( i+1 < n )) && [[ "${data:$((i+1)):1}" == '"' ]]; then
                    current_field="${current_field}\""
                    (( i++ ))
                else
                    # End of quoted field
                    state="after_quote"
                fi
            elif [[ "$ch" == $'\n' ]]; then
                current_field="${current_field}\\n"
            elif [[ "$ch" == $'\r' ]]; then
                # CR inside quoted field
                if (( i+1 < n )) && [[ "${data:$((i+1)):1}" == $'\n' ]]; then
                    current_field="${current_field}\\n"
                    (( i++ ))
                else
                    current_field="${current_field}\\n"
                fi
            elif [[ "$ch" == $'\t' ]]; then
                current_field="${current_field}\\t"
            else
                current_field="${current_field}${ch}"
            fi
        elif [[ "$state" == "after_quote" ]]; then
            if [[ "$ch" == ',' ]]; then
                current_record="${current_record}${current_field}"$'\t'
                current_field=""
                state="field"
            elif [[ "$ch" == $'\n' || "$ch" == $'\r' ]]; then
                current_record="${current_record}${current_field}"
                printf '%s\n' "$current_record"
                current_record=""
                current_field=""
                in_record=0
                if [[ "$ch" == $'\r' ]]; then
                    (( i++ ))
                fi
                state="start"
            else
                # Unexpected character after closing quote
                # Treat as error or just append?
                current_field="${current_field}${ch}"
                state="field"
            fi
        fi
    done

    # Handle the last record if there's no trailing newline
    if [[ -n "$current_record" || -n "$current_field" || "$in_record" == 1 ]]; then
        if [[ -n "$current_field" ]]; then
            current_record="${current_record}${current_field}"
        fi
        printf '%s\n' "$current_record"
    fi
}