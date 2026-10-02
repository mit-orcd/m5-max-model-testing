#!/usr/bin/env bash
find_dupes() {
    local dir="$1"
    if [[ ! -d "$dir" ]]; then
        printf '%s: %s\n' "$0" "argument is not a directory" >&2
        return 1
    fi

    local tmp tmp_sorted out
    tmp=$(mktemp -t find_dupes.XXXXXX) || return 1
    tmp_sorted=$(mktemp -t find_dupes.XXXXXX) || { rm -f "$tmp"; return 1; }
    out=$(mktemp -t find_dupes.XXXXXX) || { rm -f "$tmp" "$tmp_sorted"; return 1; }

    trap 'rm -f "$tmp" "$tmp_sorted" "$out"' EXIT

    # Gather hash and path pairs
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        local h
        h=$(md5 -q "$file") || continue
        printf '%s\t%s\n' "$h" "$file" >> "$tmp"
    done

    # Sort by hash
    sort -k1,1 "$tmp" > "$tmp_sorted"

    local cur_hash
    local -a paths
    while IFS=$'\t' read -r h p; do
        if [[ -n "$cur_hash" && "$h" != "$cur_hash" ]]; then
            if (( ${#paths[@]} >= 2 )); then
                printf '%s\n' "$(printf '%s\n' "${paths[@]}" | sort)" >> "$out"
            fi
            paths=()
        fi
        cur_hash="$h"
        paths+=("$p")
    done < "$tmp_sorted"

    # Handle last group
    if (( ${#paths[@]} >= 2 )); then
        printf '%s\n' "$(printf '%s\n' "${paths[@]}" | sort)" >> "$out"
    fi

    # Output groups sorted by first path
    sort -k1,1 "$out"
}