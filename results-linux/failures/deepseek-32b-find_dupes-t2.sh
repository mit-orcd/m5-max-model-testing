function find_dupes {
    if [ $# -ne 1 ]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    local dir="$1"
    declare -A hash_map

    while IFS= read -r file; do
        hash=$(md5 -r "$file")
        hash_map["$hash"]+="$file\n"
    done < <(find "$dir" -type f)

    local groups=()
    for hash in "${!hash_map[@]}"; do
        content="${hash_map[$hash]}"
        files=()
        while IFS= read -r line; do
            if [ -n "$line" ]; then
                files+=("$line")
            fi
        done <<< "$content"

        if [ "${#files[@]}" -ge 2 ]; then
            IFS=$'\n' sorted_files=($(sort <<< "${files[*]}"))
            group=$(printf "%s " "${sorted_files[@]}")
            group=${group% }
            groups+=("$group")
        fi
    done

    IFS=$'\n' sorted_groups=($(printf "%s\n" "${groups[@]}" | sort -k1,1))

    for group in "${sorted_groups[@]}"; do
        echo "$group"
    done
}