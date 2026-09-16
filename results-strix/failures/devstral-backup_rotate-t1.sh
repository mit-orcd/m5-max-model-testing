backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=( $(find "$dir" -maxdepth 1 -name 'backup-*.tar.gz' -printf '%f\n' | sort -r) )
    local deleted=()
    local count=${#files[@]}

    if [[ $count -gt $keep ]]; then
        for (( i=keep; i<count; i++ )); do
            deleted+=("${files[$i]}")
        done
        printf '%s\n' "${deleted[@]}" | sort
        rm -f "${deleted[@]}"
    fi
}