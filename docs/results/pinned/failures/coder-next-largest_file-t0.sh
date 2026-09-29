largest_file() {
    find "$1" -type f -exec stat -c '%s %n' {} \; 2>/dev/null | sort -nr | head -n1 | cut -d' ' -f2-
}