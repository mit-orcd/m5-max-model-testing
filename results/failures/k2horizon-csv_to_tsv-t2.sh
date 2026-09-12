csv_to_tsv() {
    local file="$1"
    local IFS=''
    local out=""
    local in_record=0
    local in_field=0
    local in_quotes=0
    local field=""
    local record=""
    local c prev
    local line=0

    # Read the file byte-wise to handle arbitrary content
    while IFS= read -r -n1 c || [[ -n "$c" ]]; do
        # Normalize CRLF: strip CR before LF
        if [[ "$c" == $'\r' ]]; then
            prev=$'\r'
            continue
        fi

        if [[ -n "$prev" && "$prev" == $'\r' ]]; then
            prev=''
            # Previous char was CR; this char is the first non-CR after it.
            # If it's LF, skip (already handled CR); otherwise process it.
            if [[ "$c" == $'\n' ]]; then
                # This is the LF of CRLF; field/row processing below
                :
            fi
        fi
        prev=''

        if (( in_quotes )); then
            if [[ "$c" == '"' ]]; then
                # Peek next char
                local next
                IFS= read -r -n1 next || next=''
                if [[ "$next" == '"' ]]; then
                    field+='"'
                    prev='"'

                    # If next is the second quote and following is a delimiter,
                    # consume it; otherwise it's part of an escaped quote.
                    if [[ -n "$next" ]]; then
                        prev=''
                        # We already consumed next; process it via recursion not possible.
                        # Instead, handle next by setting a flag is complex; we'll reprocess.
                        # Simpler: store next and continue loop with a small buffer.
                        # We'll use a small state machine approach below.
                    fi
                else
                    # End of quoted section
                    in_quotes=0
                    if [[ -n "$next" ]]; then
                        prev=''
                        # Process next as the next character in the main loop
                        # by pushing it back: we can't push back with read.
                        # Use a small buffer variable.
                        c="$next"
                        # Re-enter processing for c
                        # We'll handle it by a recursive call simulation
                        # Instead, set a pending variable.
                        pending="$next"
                        # Actually we can just loop with a pending variable.
                    fi
                fi
            else
                # Inside quotes: literal char, escape special ones
                case "$c" in
                    $'\n') field+='\\n' ;;
                    $'\t') field+='\\t' ;;
                    *) field+="$c" ;;
                esac
            fi
        else
            case "$c" in
                '"')
                    if [[ -z "$field" && $record == "$record" ]]; then
                        # Start of quoted field
                        in_quotes=1
                    else
                        field+='"'
                    fi
                    ;;
                ',')
                    record+=$'\t'
                    field=''
                    ;;
                $'\n')
                    # End of record
                    if (( in_record )); then
                        record+="$field"
                        printf '%s\n' "$record"
                        record=''
                        field=''
                        in_record=1
                    fi
                    ;;
                *)
                    field+="$c"
                    ;;
            esac
        fi
    done < "$file"
}