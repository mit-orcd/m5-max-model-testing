csv_to_tsv() {
    tr -d '\"' < "$1" | sed 's/[^\t]*\([^\t]*\)[^\t]*/\1\t/g' | sed 's/^[^\t]*//' | tr -d '\r'
}