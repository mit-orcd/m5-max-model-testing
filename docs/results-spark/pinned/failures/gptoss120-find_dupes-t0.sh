find_dupes() {
    local dir=$1
    [[ -z $dir ]] && return

    # associative array: hash -> NUL‑separated list of paths
    declare -A hash_map

    # Gather hashes
    while IFS= read -r -d '' file; do
        # md5 -q prints only the hash on macOS
        local h
        h=$(md5 -q "$file")
        hash_map["$h"]+=$'\0'"$file"
    done < <(find "$dir" -type f -print0 2>/dev/null)

    # Build groups of duplicates
    local -a groups
    for h in "${!hash_map[@]}"; do
        # Split NUL‑separated list into an array
        IFS=$'\0' read -r -d '' -a paths <<< "${hash_map[$h]}"

        # Remove possible empty first element (from leading NUL)
        local -a real_paths=()
        for p in "${paths[@]}"; do
            [[ -n $p ]] && real_paths+=("$p")
        done

        # Keep only groups with 2 or more files
        (( ${#real_paths[@]} > 1 )) || continue

        # Sort the paths within the group
        IFS=$'\n' sorted=($(printf '%s\n' "${real_paths[@]}" | sort))

        # Join them with spaces and store
        groups+=("${sorted[*]}")
    done

    # Output groups sorted by their first path
    printf '%s\n' "${groups[@]}" | sort -k1,1
}