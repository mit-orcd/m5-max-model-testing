find_dupes() {
    local dir="$1"
    local -A md5map=()
    local file
    while IFS= read -r -d '' file; do
        local hash
        hash=$(md5 -r "$file" | awk '{print $1}')
        if [[ -n "${md5map[$hash]}" ]]; then
            md5map[$hash]+=" "$file
        else
            md5map[$hash]="$file"
        fi
    done < <(find "$dir" -type f -print0 | sort -z)

    local groups=()
    for hash in "${!md5map[@]}"; do
        local files=(${md5map[$hash]})
        if (( ${#files[@]} >= 2 )); then
            local sorted_files
            sorted_files=$(printf '%s\n' "${files[@]}" | sort)
            local line
            line=$(printf '%s ' $sorted_files)
            line="${line% }"
            groups+=("$line")
        fi
    done

    if (( ${#groups[@]} > 0 )); then
        printf '%s\n' "${groups[@]}" | sort
    fi
}