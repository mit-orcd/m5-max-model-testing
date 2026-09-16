csv_to_tsv() {
    local file="$1"
    local IFS= read -r -d '' -n 1 char || true
    local state=0
    local field=""
    local line=""
    local in_field=0
    local in_quote=0
    local prev_char=""
    local crlf=0

    # Read entire file into a variable to handle multi-line fields easily
    # Since we can't use python/perl etc, we use bash's read with -d '' to slurp
    # But reading char by char is safer for state machine.
    # However, bash read -n 1 is slow. Let's try to process line by line but handle quotes.
    # Actually, RFC4180 allows newlines inside quotes. So we must read char by char or use a trick.
    # Given constraints, a state machine reading char by char is the most robust pure bash approach.

    local input
    input=$(cat "$file")
    # Remove trailing CR if present (CRLF -> LF)
    input="${input%$'\r'}"
    # Replace CRLF with LF inside the string if any remain (though cat might have stripped them depending on env)
    # Better: normalize line endings first
    input=$(printf '%s' "$input" | tr -d '\r')

    local len=${#input}
    local i=0
    local char
    local field=""
    local output_line=""
    local in_quotes=0
    local prev_char=""

    while [ $i -lt $len ]; do
        char="${input:$i:1}"
        prev_char="${input:$((i-1)):1}"

        if [ $in_quotes -eq 0 ]; then
            if [ "$char" = '"' ]; then
                in_quotes=1
            elif [ "$char" = $'\n' ]; then
                # End of record
                # Escape tabs in field
                field="${field//$'\t'/\\t}"
                output_line+="$field"$'\t'
                # Remove trailing tab if it's the last field (we add tabs between fields)
                # Actually, we build the line by appending fields.
                # Let's restructure: collect fields, then join.
                # But we need to handle the newline immediately.
                # Let's print the accumulated line and reset.
                # Wait, the logic above appends tab after field.
                # We need to handle the last field correctly.
                # Let's restart the logic with a cleaner state machine.
                break
            elif [ "$char" = ',' ]; then
                field="${field//$'\t'/\\t}"
                output_line+="$field"$'\t'
                field=""
            else
                field+="$char"
            fi
        else
            # Inside quotes
            if [ "$char" = '"' ]; then
                # Check for escaped quote ""
                local next_char="${input:$((i+1)):1}"
                if [ "$next_char" = '"' ]; then
                    field+='"'
                    ((i++))
                else
                    in_quotes=0
                fi
            elif [ "$char" = $'\n' ]; then
                # Newline inside quotes -> escape as \n
                field+="\\n"
            elif [ "$char" = $'\t' ]; then
                # Tab inside quotes -> escape as \t
                field+="\\t"
            else
                field+="$char"
            fi
        fi
        ((i++))
    done

    # This approach is getting messy with the loop. Let's use a simpler state machine with a buffer.
    # Re-implementing cleanly.
    :
}

# Correct implementation
csv_to_tsv() {
    local file="$1"
    local content
    content=$(cat "$file" | tr -d '\r')
    local len=${#content}
    local i=0
    local field=""
    local in_quotes=0
    local output=""
    local char next_char

    while [ $i -lt $len ]; do
        char="${content:$i:1}"
        next_char="${content:$((i+1)):1}"

        if [ $in_quotes -eq 0 ]; then
            if [ "$char" = '"' ]; then
                in_quotes=1
            elif [ "$char" = ',' ]; then
                # Escape tabs in field
                field="${field//$'\t'/\\t}"
                output+="$field"$'\t'
                field=""
            elif [ "$char" = $'\n' ]; then
                field="${field//$'\t'/\\t}"
                output+="$field"$'\n'
                field=""
            else
                field+="$char"
            fi
        else
            if [ "$char" = '"' ]; then
                if [ "$next_char" = '"' ]; then
                    field+='"'
                    ((i++))
                else
                    in_quotes=0
                fi
            elif [ "$char" = $'\n' ]; then
                field+="\\n"
            elif [ "$char" = $'\t' ]; then
                field+="\\t"
            else
                field+="$char"
            fi
        fi
        ((i++))
    done

    # Handle last field if no trailing newline
    if [ -n "$field" ] || [ -n "$output" ]; then
        field="${field//$'\t'/\\t}"
        # Remove trailing tab if present (from last comma)
        if [[ "$output" == *$'\t' ]]; then
            output="${output%$'\t'}"
        fi
        output+="$field"$'\n'
    fi

    printf '%s' "$output"
}