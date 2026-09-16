total_size() {
    (cd "$1" && find . -type f -exec stat -f "%z" {} + | awk '{s+=$1} END {print s+0}')
}