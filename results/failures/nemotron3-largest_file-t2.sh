largest_file() {
    find "$1" -type f -printf '%s %p\0' | sort -zn | tail -zn1 | cut -z -d' ' -f2- | tr -d '\0'
}