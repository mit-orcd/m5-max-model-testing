find_dupes() {
    # default to current directory if none supplied
    local dir="${1:-.}"
    # array to hold groups of duplicate files
    local -a groups cur_paths
    local prev_hash hash path line

    # generate "hash<TAB>path" lines, sorted by hash then path
    while IFS=$'\t' read -r hash path; do
        if [[ -n "$prev_hash" && "$hash" != "$prev_hash" ]]; then
            # end of current hash block
            if (( ${#cur_paths[@]} > 1 )); then
                groups+=("${cur_paths[*]}")
            fi
            cur_paths=()
        fi
        cur_paths+=("$path")
        prev_hash="$hash"
    done < <(
        find "$dir" -type f -print0 |
        xargs -0 md5 -r |
        sed -E 's/^([^ ]+) (.*)$/\1\t\2/' |
        sort -k1,1 -k2,2
    )

    # handle the last hash block
    if (( ${#cur_paths[@]} > 1 )); then
        groups+=("${cur_paths[*]}")
    fi

    # output groups sorted by their first path
    printf "%s\n" "${groups[@]}" | sort -k1,1
}