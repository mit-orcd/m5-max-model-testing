find_dupes() {
    local dir=$1
    if [[ -z $dir ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    # associative array: md5 => newline‑separated list of files
    declare -A md5map

    # Gather md5 hashes for all regular files under $dir
    while IFS= read -r -d '' file; do
        # md5 -r prints: "<hash> <filename>"
        hash=$(md5 -r "$file" 2>/dev/null | awk '{print $1}')
        md5map["$hash"]+=$'\n'"$file"
    done < <(find "$dir" -type f -print0 2>/dev/null)

    # Build and output groups (2+ files) sorted as required
    {
        for hash in "${!md5map[@]}"; do
            # split the stored newline‑separated list into an array
            files=()
            while IFS= read -r f; do
                [[ -n $f ]] && files+=("$f")
            done <<< "${md5map[$hash]}"

            (( ${#files[@]} < 2 )) && continue   # ignore singletons

            # sort the paths within the group
            sorted=$(printf '%s\n' "${files[@]}" | sort)

            # join with spaces and output the group line
            line=$(printf '%s ' $sorted)
            echo "${line% }"
        done
    } | sort -k1,1
}