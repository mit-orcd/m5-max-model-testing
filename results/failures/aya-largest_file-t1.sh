largest_file () {
    find "$1" -type f -exec du -a -h {} + | sort -rh | head -n 1
}