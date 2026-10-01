find_dupes() {
    local dir=$1
    [[ -d $dir ]] || return 1

    declare -A hfiles

    # Gather files and their MD5 hashes
    while IFS= read -r -d '' f; do
        local h
        h=$(md5 -r "$f" | awk '{print $1}')
        if [[ -z ${hfiles[$h]} ]]; then
            hfiles[$h]=$f
        else
            hfiles[$h]+=$'\n'"$f"
        fi
    done < <(find "$dir" -type f -print0)

    local groups=()

    # Build groups of duplicates
    for h in "${!hfiles[@]}"; do
        readarray -t files <<< "${hfiles[$h]}"
        (( ${#files[@]} < 2 )) && continue
        local line
        line=$(printf '%s\n' "${files[@]}" | sort | tr '\n' ' ' | sed 's/ $//')
        groups+=("$line")
    done

    # Output groups sorted by their first path
    printf '%s\n' "${groups[@]}" | sort -k1,1
}