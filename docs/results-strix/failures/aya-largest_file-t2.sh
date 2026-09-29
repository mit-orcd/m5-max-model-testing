function largest_file() {
    find "$1" -type f -exec du -H -c {} + | sort -rh | head -n 1
}