function backup_rotate() {
    local dir keep
    dir="$1"
    keep="$2"

    # Find the <keep> newest backup files
    newest_backup_files=$(ls -1 "$dir"/backup-*.tar.gz | head -n "$keep")

    # Find all backup files
    all_backup_files=$(ls "$dir"/backup-*.tar.gz)

    # Find the set difference between all backup files and the <keep> newest
    oldest_backup_files=$(comm -23 <(echo "$all_backup_files") <(echo "$newest_backup_files"))

    # Delete the oldest backup files and print their names
    for file in $oldest_backup_files; do
        rm "$dir"/$file
        echo "$file"
    done
}