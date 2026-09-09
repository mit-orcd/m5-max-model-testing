function backup_rotate() {
    local dir="${1:-}" keep="${2:-}"
    local files=("$dir"/backup-*.tar.gz)
    local -a to_delete=()
    local -a to_keep=()

    # sort files lexicographically (oldest first)
    IFS=$'\n' sorted_files=($(sort -V <<<"${files[*]}"))
    unset IFS

    # delete oldest files until we have 'keep' left
    for file in "${sorted_files[@]}"; do
        [[ $(( ${#to_delete[@]} + ${#to_keep[@]} )) -le $keep ]] && break
        to_delete+=("$file")
    done

    # print deleted files (old to new)
    for file in "${to_delete[@]}"; do
        echo "$file"
    done

    # move remaining files to 'to_keep' array
    shift "${to_delete[@]}"
    to_keep+=("${sorted_files[@]:0:${#to_delete[@]}}")

    # delete all files
    rm "${to_delete[@]}"

    # move remaining files back to original array
    shift "${to_keep[@]}"
    files+=("${to_keep[@]}")
}