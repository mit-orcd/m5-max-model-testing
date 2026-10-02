#!/usr/bin/env bash
find_dupes() {
    local dir="$1"
    if [[ -z "$dir" ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi
    if [[ ! -d "$dir" ]]; then
        echo "Error: '$dir' is not a directory" >&2
        return 1
    fi

    local tmp=$(mktemp) || return 1
    local tmp_sorted=$(mktemp) || { rm -f "$tmp"; return 1; }
    local out=$(mktemp) || { rm -f "$tmp" "$tmp_sorted"; return 1; }

    trap 'rm -f "$tmp" "$tmp_sorted" "$out"' EXIT

    # Build hash <tab> file list
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        local hash
        hash=$(md5 -r "$file" | awk '{print $1}')
        printf '%s\t%s\n' "$hash" "$file"
    done > "$tmp"

    # Sort by hash then by path
    sort -t$'\t' -k1,1 -k2,2 "$tmp" > "$tmp_sorted"

    local current_hash=""
    local -a files=()

    process_group() {
        local -a sorted_files=()
        if (( ${#files[@]} >= 2 )); then
            printf '%s\n' "${files[@]}" | sort > "$tmp_sorted.tmp"
            mapfile -t sorted_files < "$tmp_sorted.tmp"
            rm -f "$tmp_sorted.tmp"
            printf '%s\n' "${sorted_files[*]}" >> "$out"
        fi
    }

    while IFS=$'\t' read -r hash file; do
        if [[ -n "$current_hash" && "$hash" != "$current_hash" ]]; then
            process_group
            files=()
        fi
        current_hash="$hash"
        files+=("$file")
    done < "$tmp_sorted"

    # Process last group
    process_group

    # Output groups sorted by first path
    sort "$out"
}