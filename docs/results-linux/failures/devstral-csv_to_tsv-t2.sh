csv_to_tsv() {
    awk -F, -v OFS="\t" '
    {
        gsub(/\r/, "")  # Remove CR characters
        for (i = 1; i <= NF; i++) {
            if ($i ~ /^[\"].*[\"]$/) {
                # Remove surrounding quotes
                $i = substr($i, 2, length($i) - 2)
                # Replace escaped quotes with single quotes
                gsub(/\"{2}/, "\"", $i)
                # Replace internal newlines with \n
                gsub(/\n/, "\\n", $i)
                # Replace internal tabs with \t
                gsub(/\t/, "\\t", $i)
            }
        }
        print
    }' "$1"
    echo  # Ensure output ends with a newline
}