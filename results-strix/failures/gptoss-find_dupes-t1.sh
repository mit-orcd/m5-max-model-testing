find_dupes() {
    local dir="$1"
    if [[ -z "$dir" || ! -d "$dir" ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    local tmpfile=$(mktemp)
    local sortedfile=$(mktemp)
    local groupfile=$(mktemp)

    # Generate hash <tab> path list
    while IFS= read -r -d '' file; do
        local hash=$(md5 -q "$file")
        printf '%s\t%s\n' "$hash" "$file" >> "$tmpfile"
    done < <(find "$dir" -type f -print0)

    # Sort by hash, then path
    sort -k1,1 -k2,2 "$tmpfile" > "$sortedfile"

    local prev_hash=""
    local -a paths=()

    while IFS= read -r line; do
        local hash=${line%%$'\t'*}
        local path=${line#*$'\t'}

        if [[ "$hash" != "$prev_hash" && -n "$prev_hash" ]]; then
            if (( ${#paths[@]} >= 2 )); then
                local sorted=$(printf '%s\n' "${paths[@]}" | sort | paste -sd ' ')
                echo "$sorted" >> "$groupfile"
            fi
            paths=()
        fi
        prev_hash="$hash"
        paths+=("$path")
    done < "$sortedfile"

    # Handle the last group
    if (( ${#paths[@]} >= 2 )); then
        local sorted=$(printf '%s\n' "${paths[@]}" | sort | paste -sd ' ')
        echo "$sorted" >> "$groupfile"
    fi

    # Output groups sorted by first path
    if [[ -s "$groupfile" ]]; then
        sort "$groupfile"
    fi

    # Clean up
    rm -f "$tmpfile" "$sortedfile" "$groupfile"
}