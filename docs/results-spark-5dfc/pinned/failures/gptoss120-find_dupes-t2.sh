find_dupes() {
    local dir=$1
    [[ -d $dir ]] || return 1

    # hash => newline‑separated list of files
    declare -A hash_files

    # Gather files and compute MD5 hashes (macOS `md5 -r`)
    while IFS= read -r -d '' file; do
        # md5 -r prints: "<hash> <filename>"
        local hash
        hash=$(md5 -r "$file" | awk '{print $1}')
        if [[ -z ${hash_files[$hash]} ]]; then
            hash_files[$hash]="$file"
        else
            hash_files[$hash]="${hash_files[$hash]}"$'\n'"$file"
        fi
    done < <(find "$dir" -type f -print0)

    local groups=()

    # Build groups of duplicates
    for hash in "${!hash_files[@]}"; do
        # Split the newline‑separated list into an array
        mapfile -t files <<< "${hash_files[$hash]}"
        (( ${#files[@]} < 2 )) && continue   # need at least two files

        # Sort the paths within the group
        IFS=$'\n' sorted=($(printf "%s\n" "${files[@]}" | sort))

        # Join them space‑separated
        groups+=("$(printf "%s " "${sorted[@]}" | sed 's/ $//')")
    done

    # Output groups sorted by their first path
    printf "%s\n" "${groups[@]}" | sort
}