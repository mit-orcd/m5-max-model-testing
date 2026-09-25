find_dupes() {
    local dir=$1
    if [[ -z $dir ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    # temporary file for hash‑path pairs
    local tmp
    tmp=$(mktemp) || return

    # generate "<md5> <path>" lines for every regular file
    find "$dir" -type f -print0 2>/dev/null |
        while IFS= read -r -d '' file; do
            md5 -r "$file" 2>/dev/null
        done > "$tmp"

    local prev_hash="" path
    local -a paths group_lines

    # sort by hash, then collect groups
    while IFS= read -r hash path; do
        if [[ $hash != $prev_hash && $prev_hash != "" ]]; then
            if (( ${#paths[@]} > 1 )); then
                sorted=$(printf '%s\n' "${paths[@]}" | LC_ALL=C sort | tr '\n' ' ')
                group_lines+=("${sorted% }")
            fi
            paths=()
        fi
        paths+=("$path")
        prev_hash=$hash
    done < <(LC_ALL=C sort "$tmp")

    # handle the final group
    if (( ${#paths[@]} > 1 )); then
        sorted=$(printf '%s\n' "${paths[@]}" | LC_ALL=C sort | tr '\n' ' ')
        group_lines+=("${sorted% }")
    fi

    # output groups sorted by their first path
    printf '%s\n' "${group_lines[@]}" | LC_ALL=C sort -k1,1

    rm -f "$tmp"
}