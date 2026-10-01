function backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=("$dir"/backup-*.tar.gz)
    local -a to_delete=()
    local -a to_keep=()

    # sort files oldest to newest
    sorted_files=($(for file in "${files[@]}"; do echo "$file"; done | sort -r))

    # delete oldest files until we have 'keep' number of files left
    for file in "${sorted_files[@]:0:$((${#sorted_files[@]} - $keep))}"; do
        to_delete+=("$file")
    done

    # move remaining files to 'to_keep' array
    for file in "${sorted_files[@]:$((${#sorted_files[@]} - $keep))}:$keep"; do
        to_keep+=("$file")
    done

    # delete the files
    for file in "${to_delete[@]}"; do
        echo "$file"
        rm "$file"
    done

    # print the remaining files
    for file in "${to_keep[@]}"; do
        echo "$file"
    done
}