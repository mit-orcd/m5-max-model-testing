csv_to_tsv() {
    awk -F',' -v OFS='\t' '
    {
        gsub(/\r/, "")  # Remove CR
        for (i = 1; i <= NF; i++) {
            if ($i ~ /^".*"$/) {
                gsub(/\"/, "\"\"", $i)  # Escape internal quotes
                gsub(/\n/, "\\n", $i)  # Escape newlines
                gsub(/\t/, "\\t", $i)  # Escape tabs
                $i = substr($i, 2, length($i) - 2)  # Remove surrounding quotes
            }
        }
        print
    }
    ' "$1"
    printf '\n'  # Ensure trailing newline
}