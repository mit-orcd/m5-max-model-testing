function backup_rotate() {
    local dir="$1"
    local keep="$2"

    if [[ $# -ne 2 || ! $dir || ! $keep ]]; then
        echo "Usage: backup_rotate <dir> <keep>"
        return 1
    fi

    local files=("$dir"/backup-*.tar.gz)
    if [[ ${#files[@]} -le $keep ]]; then
        return 0
    fi

    local oldest_file=${files[$(($#files - $keep))]}
    local oldest_name=$(basename "$oldest_file")

    for file in "${files[@]:$((($#files - $keep) + 1))}"; do
        local name=$(basename "$file")
        echo "$name"
        rm "$file"
    done

    echo "$oldest_name"
    rm "$oldest_file"
}