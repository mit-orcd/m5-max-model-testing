csv_to_tsv() {
    local file="$1"
    awk '
    BEGIN { FPAT = "([^,]*|\"[^\"]*\")"; OFS = "\t" }
    {
        gsub(/\r$/, "")
        for (i = 1; i <= NF; i++) {
            f = $i
            if (substr(f, 1, 1) == "\"" && substr(f, length(f), 1) == "\"") {
                f = substr(f, 2, length(f) - 2)
                gsub(/""/, "\"", f)
                gsub(/\n/, "\\n", f)
                gsub(/\t/, "\\t", f)
            } else {
                gsub(/\n/, "\\n", f)
                gsub(/\t/, "\\t", f)
            }
            $i = f
        }
        print
    }
    ' "$file"
}