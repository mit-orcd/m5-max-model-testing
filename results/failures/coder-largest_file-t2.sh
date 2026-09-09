largest_file() {
    find "$1" -type f -exec stat -c "%s %n" {} \; | sort -n | tail -1 | cut -d' ' -f2-
}