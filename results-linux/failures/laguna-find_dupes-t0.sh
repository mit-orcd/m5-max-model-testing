find_dupes() {
    local dir="$1"
    local -A hash_files=()
    local -a all_hashes=()

    while IFS= read -r -d '' file; do
        if [[ -f "$file" ]]; then
            hash=$(md5 -r "$file" | cut -d' ' -f1)
            hash_files["$hash"]+=" $file"
            all_hashes+=("$hash")
        fi
    done < <(find "$dir" -type f -print0)

    local -A seen_hashes=()
    for hash in "${all_hashes[@]}"; do
        if [[ -z "${seen_hashes[$hash]}" ]]; then
            seen_hashes["$hash"]=1
            files="${hash_files[$hash]}"
            file_count=$(echo "$files" | wc -w)
            if (( file_count >= 2 )); then
                sorted_files=$(echo "$files" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//')
                echo "$sorted_files"
            fi
        fi
    done
}