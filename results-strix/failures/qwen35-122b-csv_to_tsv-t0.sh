csv_to_tsv() {
    local file="$1"
    local IFS=
    local -a fields
    local field=""
    local in_quotes=0
    local char
    local output=""
    local first_field=1
    local cr_seen=0

    while IFS= read -r -d '' -n 1 char || [[ -n "$char" ]]; do
        if [[ "$char" == $'\r' ]]; then
            cr_seen=1
            continue
        fi
        if [[ "$char" == $'\n' ]]; then
            if [[ $in_quotes -eq 1 ]]; then
                field+='\n'
            else
                if [[ $first_field -eq 0 ]]; then
                    output+=$'\t'
                fi
                output+="$field"
                first_field=0
                if [[ $cr_seen -eq 1 ]]; then
                    cr_seen=0
                fi
                printf '%s\n' "$output"
                output=""
                field=""
                first_field=1
            fi
            continue
        fi
        if [[ "$char" == '"' ]]; then
            if [[ $in_quotes -eq 1 ]]; then
                # Check for escaped quote ""
                # We need to peek ahead, but read -n 1 consumes one by one.
                # Strategy: if next char is ", consume it and add one ".
                # Since we can't peek easily in a loop like this without buffering,
                # we handle the logic by checking if we are at the end of a field or not.
                # Actually, standard RFC 4180: "" inside quotes is a quote.
                # We need to look at the next character.
                # Let's restructure slightly to handle the peek.
                :
            fi
            # Toggle quote state
            if [[ $in_quotes -eq 0 ]]; then
                in_quotes=1
            else
                in_quotes=0
            fi
            continue
        fi

        if [[ $in_quotes -eq 1 ]]; then
            if [[ "$char" == '"' ]]; then
                # This is the start of a potential "" sequence.
                # We need to check the next char.
                # Since we are in a loop reading one char, we can't easily peek.
                # Let's use a different approach: read the whole file into a variable?
                # No, file might be large.
                # Let's use a state machine with a lookahead buffer.
                :
            fi
            field+="$char"
        else
            if [[ "$char" == ',' ]]; then
                if [[ $first_field -eq 0 ]]; then
                    output+=$'\t'
                fi
                output+="$field"
                field=""
                first_field=0
            else
                field+="$char"
            fi
        fi
    done < "$file"

    # Handle last record if no trailing newline
    if [[ -n "$field" ]] || [[ $first_field -eq 0 ]]; then
        if [[ $first_field -eq 0 ]]; then
            output+=$'\t'
        fi
        output+="$field"
        printf '%s\n' "$output"
    fi
}