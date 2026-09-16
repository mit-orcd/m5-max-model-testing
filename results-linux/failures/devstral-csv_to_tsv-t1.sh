csv_to_tsv() {
    awk -F'["]' -v OFS='\t' '
    {
        line = $0
        # Handle quoted fields
        while (match(line, /^("[^"]*([^"]"[^"]*)*")/, m)) {
            field = m[1]
            gsub(/\r/, "", field)
            gsub(/\n/, "\\n", field)
            gsub(/\t/, "\\t", field)
            gsub(/""/, "\"", field)
            sub(/^"|"$/, "", field)
            line = substr(line, RLENGTH)
            printf "%s%s", field, OFS
        }
        # Handle unquoted fields
        while (match(line, /^[^,]*/, m)) {
            field = m[0]
            gsub(/\r/, "", field)
            printf "%s%s", field, OFS
            line = substr(line, RLENGTH)
            if (line ~ /^,$/) {
                printf "%s", OFS
                line = substr(line, 2)
            }
        }
        if (line ~ /^,$/) {
            printf "%s", OFS
            line = substr(line, 2)
        }
        printf "\n"
    }
    ' "$1"
}