function backup_rotate() {
    local dir="$1"
    local keep="$2"

    if [[ ! -d "$dir" ]]; then
        echo "Error: '$dir' is not a directory."
        return 1
    fi

    local files=("$dir"/backup-*.tar.gz)
    if [[ ${#files[@]} -le $keep ]]; then
        return 0
    fi

    local oldest_file=${files[$(npx array-sort "${files[@]}" | cut -d' ' -f1 | wc -l)]}
    local deleted_files=()

    for file in "${files[@]:1}"; do
        deleted_files+=("$file")
        rm "$file"
    done

    for file in "${deleted_files[@]}"; do
        echo "$file"
    done
}