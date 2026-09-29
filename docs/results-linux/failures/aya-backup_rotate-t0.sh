function backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=("$dir"/backup-*.tar.gz)
    local -a to_delete=()
    local -a to_keep=()

    # sort files oldest to newest
    sorted_files=($(for file in "${files[@]}"; do echo "$file"; done | sort -r))

    # delete oldest files until we have 'keep' left
    for file in "${sorted_files[@]:0:$((${#sorted_files[@]} - $keep))}"; do
        to_delete+=("$file")
    done

    # print and delete
    for file in "${to_delete[@]}"; do
        echo "$file"
        rm "$file"
    done
}