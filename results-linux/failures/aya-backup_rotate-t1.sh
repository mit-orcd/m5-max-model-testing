function backup_rotate() {
    local dir="$1"
    local keep="$2"

    # validate inputs
    if [[ ! -d "$dir" || "$keep" -lt 1 ]]; then
        echo "Usage: backup_rotate <dir> <keep>"
        return 1
    fi

    # find the files to keep
    latest_files=()
    while IFS= read -r file; do
        latest_files+=("$file")
    done < <(find "$dir" -type f -name 'backup-*.tar.gz' | sort -r | head -n "$keep")

    # delete the rest
    for file in "${latest_files[@]:0:$((${#latest_files[@]} - keep))}"; do
        echo "$file"
        rm "$file"
    done
}