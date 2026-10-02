function backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=("$dir"/backup-*.tar.gz)
    local -a to_delete=()
    local -a to_keep=()

    # sort files oldest to newest
    IFS=$'\n' sorted_files=($(sort -r <<<"${files[*]}"))
    unset IFS

    # delete all but the newest <keep> files
    for file in "${sorted_files[@]:keep:1}"; do
        to_delete+=("$file")
    done

    # print deleted files oldest first
    for file in "${to_delete[@]}"; do
        echo "$file"
    done

    # delete files
    for file in "${to_delete[@]}"; do
        rm "$file"
    done
}