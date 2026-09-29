csv_to_tsv() {
    awk -v FPAT='([^,]+)|(\"[^\"]*(\"\")[^\"]*\")' -v OFS='\t' '
    {
        for (i=1; i<=NF; i++) {
            # Unquote the field
            gsub(/""/, "\"", $i)
            if ($i ~ /^\"[^\"]*\"$/) {
                $i = substr($i, 2, length($i)-2)
                # Replace CRLF and LF with \n, tabs with \t
                gsub(/\r?\n/, "\\n", $i)
                gsub(/\t/, "\\t", $i)
            }
        }
        print
    }
    ' "$1"
    # Add trailing newline if input didn't have one
    if [ -n "$(tail -c1 "$1")" ]; then
        printf "\n"
    fi
}