function backup_rotate() {
    local dir="${1:-}" keep="${2:-}"
    local files=("$dir"/backup-*.tar.gz)
    local -a to_delete
    local -a to_keep
    local i

    if [[ $# -eq 0 || -z $dir || -z $keep || $keep -lt 1 ]]; then
        echo "Usage: backup_rotate <dir> <keep>"
        return 1
    fi

    if ! [[ $dir =~ ^/ ]] && ! [[ $dir =~ ^[a-z]:[\\/] ]]; then
        dir="$PWD/$dir"
    fi

    if ! [[ $dir =~ ^[./a-z]:[\\/] ]]; then
        echo "Invalid directory: $dir"
        return 1
    fi

    if ! [[ $keep =~ ^[0-9]+$ ]] || $keep -gt 9999; then
        echo "Invalid keep count: $keep"
        return 1
    fi

    i=0
    while [[ $i -lt ${#files[@]} ]]; do
        ((i++))
        [[ ${files[$i]} =~ ^$dir/backup-([0-9]+)-(.*)$ ]] || continue
        local date="${BASH_REMATCH[1]}"
        local name="${BASH_REMATCH[2]}"
        to_delete+=("$name")
        to_keep+=("$date-$name")
    done

    local -a sorted_keep=($(sort -r <<< "${to_keep[*]}"))
    local -a sorted_delete=($(sort -r <<< "${to_delete[*]}"))

    if [[ ${#sorted_keep[@]} -le $keep ]]; then
        echo "Nothing to delete."
        return 0
    fi

    local -a delete_list=($(setdiff "${sorted_delete[@]}" "${sorted_keep[@]:0:$keep}"))

    for file in "${delete_list[@]}"; do
        echo "Deleting: $file"
        rm "$dir/$file"
    done
}