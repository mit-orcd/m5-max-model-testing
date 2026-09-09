function find_dupes {
    local dir="$1"
    declare -A file_hashes

    while IFS= read -r file; do
        hash=$(md5 -r "$file" | cut -d ' ' -f3)
        file_hashes[$hash]="${file_hashes[$hash]} $file"
    done < <(find "$dir" -type f)

    groups=()
    for hash in "${!file_hashes[@]}"; do
        files=(${file_hashes[$hash]})
        if (( ${#files[@]} >= 2 )); then
            sorted_files=($(sort <<< "${files[@]}"))
            groups+=("$(echo "${sorted_files[@]}" | tr '\n' ' ')")
        fi
    done

    IFS=$'\n' sorted_groups=($(sort <<< "${groups[*]}"))
    unset IFS

    for group in "${sorted_groups[@]}"; do
        echo "$group"
    done
}