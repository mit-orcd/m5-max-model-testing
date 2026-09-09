find_dupes() {
    local dir="$1"
    [[ -z "$dir" || ! -d "$dir" ]] && return 1

    local -a files=()
    while IFS= read -r -d '' f; do
        files+=("$f")
    done < <(find "$dir" -type f -print0)

    (( ${#files[@]} -eq 0 )) && return 0

    declare -A md5_map=()

    local f hash
    for f in "${files[@]}"; do
        hash=$(md5 -r "$f" | awk '{print $1}')
        if [[ -n "${md5_map[$hash]+x}" ]]; then
            md5_map["$hash"]+=" $f"
        else
            md5_map["$hash"]="$f"
        fi
    done

    local -a groups=()
    local key
    for key in "${!md5_map[@]}"; do
        local -a paths=()
        read -ra paths <<< "${md5_map[$key]}"
        if (( ${#paths[@]} >= 2 )); then
            local sorted
            sorted=$(printf '%s\n' "${paths[@]}" | sort)
            groups+=("$sorted")
        fi
    done

    (( ${#groups[@]} -eq 0 )) && return 0

    printf '%s\n' "${groups[@]}" | sort
}