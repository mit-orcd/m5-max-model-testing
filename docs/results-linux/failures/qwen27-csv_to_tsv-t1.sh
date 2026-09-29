csv_to_tsv() {
    local file="$1"
    local line
    local record=""
    local field=""
    local in_quotes=0
    local i=0
    local c
    local len
    local prev_was_quote=0

    # Read the file character by character
    # First, read the entire file into a variable, handling CRLF
    local content
    content=$(cat -- "$file" 2>/dev/null)
    # Ensure content ends with a newline for consistent processing
    # Actually, let's process line by line but handle quoted fields spanning lines

    # We need to process the file properly. Let's use a while read loop but
    # we need to handle quoted fields that span multiple lines.
    # Better approach: read the whole file and process character by character.

    # Read file into a variable, preserving all characters
    local data
    data=$(<"$file")
    # data may have trailing newlines stripped by $(). Let's use a different approach.
    # Use process substitution or read all bytes.

    # Let's use a while loop reading byte by byte from the file
    local out=""
    local cur_field=""
    local in_field=0
    local in_q=0
    local prev_c=""

    # Read all bytes
    local bytes
    bytes=$(cat -- "$file" | od -An -tx1 | tr -d ' \n')

    # Process each byte pair
    local j=0
    local total=$(( ${#bytes} / 2 ))

    for (( j=0; j<total; j++ )); do
        local hex=${bytes:$(($j*2)):2}
        local dec=$((16#$hex))

        # Convert to character
        local ch
        if (( dec == 13 )); then
            ch=$'\r'
        elif (( dec == 10 )); then
            ch=$'\n'
        elif (( dec == 9 )); then
            ch=$'\t'
        elif (( dec == 34 )); then
            ch='"'
        elif (( dec == 44 )); then
            ch=','
        else
            ch=$(printf "\\x$(printf '%02x' $dec)")
        fi

        # State machine
        if (( in_q )); then
            if [[ "$ch" == '"' ]]; then
                if [[ "$prev_c" == '"' ]]; then
                    # Escaped quote: "" -> "
                    cur_field+='"'
                    prev_c=""
                else
                    # End of quoted field
                    in_q=0
                    prev_c=""
                fi
            else
                if [[ "$ch" == $'\n' ]]; then
                    cur_field+='\n'
                elif [[ "$ch" == $'\t' ]]; then
                    cur_field+='\t'
                elif [[ "$ch" == $'\r' ]]; then
                    # Skip CR
                    :
                else
                    cur_field+="$ch"
                fi
                prev_c="$ch"
            fi
        else
            # Not in quotes
            if [[ "$ch" == '"' ]]; then
                if [[ -z "$cur_field" && $in_field -eq 0 ]]; then
                    # Start of quoted field
                    in_q=1
                    in_field=1
                    prev_c=""
                elif [[ "$prev_c" == '"' ]]; then
                    # Escaped quote in unquoted? No, this shouldn't happen.
                    # Actually, "" outside quotes is just a quote? No, per RFC 4180,
                    # quotes only have special meaning at the start of a field.
                    cur_field+='"'
                    prev_c=""
                else
                    # Quote in middle of unquoted field - treat as literal
                    cur_field+='"'
                    in_field=1
                    prev_c="$ch"
                fi
            elif [[ "$ch" == ',' ]]; then
                # End of field
                out+="${cur_field}"
                out+=$'\t'
                cur_field=""
                in_field=0
                prev_c=""
            elif [[ "$ch" == $'\n' ]]; then
                # End of record
                out+="${cur_field}"
                out+=$'\n'
                cur_field=""
                in_field=0
                prev_c=""
            elif [[ "$ch" == $'\r' ]]; then
                # Skip CR (will be followed by LF)
                prev_c=""
            else
                cur_field+="$ch"
                in_field=1
                prev_c="$ch"
            fi
        fi
    done

    # If we ended in quotes, that's malformed, but let's flush
    if (( in_q )); then
        out+="${cur_field}"
    fi

    # If there's a pending field (no trailing newline), add it
    if [[ -n "$cur_field" || $in_field -eq 1 ]]; then
        out+="${cur_field}"
        out+=$'\n'
    fi

    printf '%s' "$out"
}