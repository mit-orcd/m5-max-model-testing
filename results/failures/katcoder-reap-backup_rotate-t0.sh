backup_rotate() {
    local dir="$1"
    local keep="$2"
    local files=()
    local f
    for f in "$dir"/backup-*.tar.gz; do
        [ -e "$f" ] || continue
        files+=("$(basename "$f")")
    done
    if (( ${#files[@]} <= keep )); then
        return
    fi
    IFS=$'\n' sorted=($(printf '%s\n' "${files[@]}" | sort)); unset IFS
    local start=$(( ${#sorted[@]} - keep ))
    for (( i=start; i<${#sorted[@]}; i++ )); do
        echo "${sorted[i]}"
        rm -f "$dir/${sorted[i]}"
    done
}