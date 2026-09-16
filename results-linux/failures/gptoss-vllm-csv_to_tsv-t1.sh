#!/usr/bin/env bash
csv_to_tsv() {
    local file="$1"
    local data
    # Read entire file, preserving newlines (but stripping CRs later)
    IFS=; if ! read -r -d '' data < "$file" 2>/dev/null; then
        data=$(cat "$file")
    fi

    local len=${#data}
    local i=0
    local in_quote=0
    local field=""
    local -a fields=()
    local record_output=0
    local last_char=""

    while [ $i -lt $len ]; do
        local c=${data:i:1}
        last_char="$c"

        # Skip CR characters
        if [ "$c" = $'\r' ]; then
            i=$((i+1))
            continue
        fi

        if [ $in_quote -eq 1 ]; then
            if [ "$c" = '"' ]; then
                local next=${data:i+1:1}
                if [ "$next" = '"' ]; then
                    field+='"'
                    i=$((i+2))
                    continue
                else
                    in_quote=0
                    i=$((i+1))
                    continue
                fi
            elif [ "$c" = $'\n' ]; then
                field+="\\n"
            elif [ "$c" = $'\t' ]; then
                field+="\\t"
            else
                field+="$c"
            fi
        else
            if [ "$c" = ',' ]; then
                fields+=("$field")
                field=""
            elif [ "$c" = $'\n' ]; then
                fields+=("$field")
                # emit record
                local out
                out=$(IFS=$'\t'; printf '%s' "${fields[*]}")
                echo "$out"
                fields=()
                field=""
                record_output=1
            elif [ "$c" = '"' ]; then
                in_quote=1
            else
                field+="$c"
            fi
        fi
        i=$((i+1))
    done

    # Handle last record if not already output
    if [ $record_output -eq 0 ]; then
        if [ "$last_char" != ',' ] && [ "$last_char" != $'\n' ]; then
            fields+=("$field")
        fi
        if [ ${#fields[@]} -gt 0 ] || [ -n "$field" ]; then
            local out
            out=$(IFS=$'\t'; printf '%s' "${fields[*]}")
            echo "$out"
        fi
    fi
}