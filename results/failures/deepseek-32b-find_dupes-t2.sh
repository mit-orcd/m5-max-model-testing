function find_dupes() {
    local dir=$1
    declare -A md5_map

    find "$dir" -type f -exec md5 {} \; | while IFS= read -r line; do
        hash=$(echo "$line" | cut -d ' ' -f1)
        filename=$(echo "$line" | cut -d ' ' -f2-)
        md5_map[$hash]="${md5_map[$hash]} $filename"
    done

    declare -a groups
    for hash in "${!md5_map[@]}"; do
        files=(${md5_map[$hash]})
        if [ ${#files[@]} -ge 2 ]; then
            IFS=$'\n' sorted_files=($(sort <<< "${files[*]}"))
            group=$(echo "${sorted_files[@]}" | tr ' ' '\n' | sort | tr '\n' ' ')
            groups+=("$group")
        fi
    done

    IFS=$'\n' sorted_groups=($(sort -k1 <<< "${groups[*]}"))
    for group in "${sorted_groups[@]}"; do
        echo "$group"
    done
}