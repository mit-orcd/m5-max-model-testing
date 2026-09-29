function backup_rotate () {
    local dir="$1"
    local keep="$2"
    local files=("$dir"/*backup-*.tar.gz)
    local i=0
    local n=${#files[@]}
    while [ $i -lt $n - $keep ]; do
        local file=${files[$i]}
        echo "$file"
        rm "$file"
        i=$((i+1))
    done
}