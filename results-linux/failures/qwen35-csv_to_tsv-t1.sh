csv_to_tsv() {
    local file="$1"
    local in_quotes=0
    local field=""
    local record=""
    local char prev_char=""
    local i
    local n

    # Read file character by character, handling CRLF and LF
    while IFS= read -r -n1 char || [[ -n "$char" ]]; do
        if [[ "$char" == $'\r' ]]; then
            if [[ "${prev_char}" == $'\n' ]]; then
                # Handle CRLF as newline, CR already consumed in previous iteration
                continue
            fi
            # Treat CR as newline if not preceded by LF (though standard is CRLF)
            char=$'\n'
        fi

        if [[ $in_quotes -eq 0 ]]; then
            if [[ "$char" == '"' ]]; then
                in_quotes=1
            elif [[ "$char" == $'\n' ]]; then
                # End of record
                printf '%s\n' "$record"
                record=""
                field=""
            elif [[ "$char" == ',' ]]; then
                # End of field
                record+="$field"
                record+=$'\t'
                field=""
            else
                field+="$char"
            fi
        else
            # Inside quotes
            if [[ "$char" == '"' ]]; then
                if [[ "$prev_char" == '"' ]]; then
                    # Literal quote handled by previous iteration's logic or current
                    # Actually, we need to check if the previous char was a quote
                    # But we are reading char by char.
                    # If we see a quote, check if it's an escaped quote ""
                    # We need to look back. Since we process sequentially:
                    # If current is " and previous was " -> it's an escaped quote.
                    # But we already appended the previous char.
                    # Let's rethink: if we see ", check if the last char added to field was "
                    # No, simpler: if we see "" sequence.
                    # If current is " and the last char read (before this) was "
                    # We need to handle this state.
                    # Let's use a different approach for escaped quotes.
                    :
                else
                    # End of quoted field
                    in_quotes=0
                fi
            elif [[ "$char" == $'\n' ]]; then
                # Newline inside quoted field -> escape to \n
                field+=$'\\n'
            elif [[ "$char" == $'\t' ]]; then
                # Tab inside quoted field -> escape to \t
                field+=$'\\t'
            else
                field+="$char"
            fi
        fi
        prev_char="$char"
    done < "$file"

    # Handle last field and record if file didn't end with newline
    if [[ -n "$field" ]] || [[ -n "$record" ]]; then
        record+="$field"
        # Remove trailing tab if it exists (empty last field case)
        if [[ "$record" == *$'\t' ]]; then
            record="${record%$'\t'}"
        fi
        printf '%s\n' "$record"
    fi
}

# Refine the logic for escaped quotes "" and proper state machine
csv_to_tsv() {
    local file="$1"
    local in_quotes=0
    local field=""
    local record=""
    local char prev_char=""
    
    # We need to handle the "" sequence carefully.
    # When in_quotes is 1:
    # If char is " and prev_char is " -> it's an escaped quote, append " to field, set prev_char to " (which it is)
    # But we need to consume the second quote.
    # Actually, standard CSV: "" inside quotes means one ".
    # So if we see "" we output one " and stay in quotes.
    
    while IFS= read -r -n1 char || [[ -n "$char" ]]; do
        # Handle CR/LF normalization
        if [[ "$char" == $'\r' ]]; then
            if [[ "${prev_char}" == $'\n' ]]; then
                # This is the CR in CRLF, already handled as LF in prev iteration?
                # No, read -n1 returns CR. Then next is LF.
                # If we see CR, treat as LF.
                char=$'\n'
            else
                # Standalone CR, treat as LF
                char=$'\n'
            fi
        fi

        if [[ $in_quotes -eq 0 ]]; then
            if [[ "$char" == '"' ]]; then
                in_quotes=1
            elif [[ "$char" == $'\n' ]]; then
                # End of record
                if [[ -n "$field" ]]; then
                    record+="$field"
                fi
                # Output record
                printf '%s\n' "$record"
                record=""
                field=""
            elif [[ "$char" == ',' ]]; then
                # End of field
                record+="$field"
                record+=$'\t'
                field=""
            else
                field+="$char"
            fi
        else
            # Inside quotes
            if [[ "$char" == '"' ]]; then
                if [[ "$prev_char" == '"' ]]; then
                    # This is the second quote in ""
                    # The previous iteration added a " to field? No, we need to be careful.
                    # If we see " and the previous char was ", it means we have ""
                    # We should have added the first " already.
                    # So we just ignore this second " and keep the first " in field.
                    # But wait, if we process char by char:
                    # i: char=" , prev="" -> field+="", prev="
                    # i+1: char=" , prev=" -> This is the escaped one.
                    # We should NOT add another ". We just skip this char effectively.
                    # But we need to set prev_char correctly? No, just skip.
                    :
                else
                    # End of quoted field
                    in_quotes=0
                fi
            elif [[ "$char" == $'\n' ]]; then
                field+=$'\\n'
            elif [[ "$char" == $'\t' ]]; then
                field+=$'\\t'
            else
                field+="$char"
            fi
        fi
        prev_char="$char"
    done < "$file"

    # Handle last record
    if [[ -n "$field" ]]; then
        record+="$field"
    fi
    if [[ -n "$record" ]]; then
        printf '%s\n' "$record"
    fi
}