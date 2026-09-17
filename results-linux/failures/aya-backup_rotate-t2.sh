function backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=("$dir"/*backup-*.tar.gz)
    if [[ ${#files[@]} -le "$keep" ]]; then
        return
    fi
    local oldest_file=${files[0]}
    local oldest_name=$(basename "$oldest_file")
    local i=1
    while [[ $i -lt ${#files[@]} ]]; do
        local file=${files[$i]}
        local name=$(basename "$file")
        if [[ $name -gt $oldest_name ]]; then
            oldest_file=$file
            oldest_name=$name
        fi
        i=$((i+1))
    done
    local deleted=("$dir"/backup-*.tar.gz)
    for file in "${deleted[@]:$((i-1))}"; do
        echo "$file"
        rm "$file"
    done
}