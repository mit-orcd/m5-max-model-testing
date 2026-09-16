csv_to_tsv() {
    awk '
    BEGIN {
        FPAT = "([^,\"]*)|(\"([^\"]|\"\")*\")"
        OFS = "\t"
    }
    {
        for (i = 1; i <= NF; i++) {
            val = $i
            if (val ~ /^\"/) {
                # Remove surrounding quotes
                val = substr(val, 2, length(val) - 2)
                # Handle escaped quotes "" -> "
                gsub(/""/, "\"", val)
                # Handle line breaks and tabs
                gsub(/\r?\n/, "\\n", val)
                gsub(/\t/, "\\t", val)
            }
            # Ensure CR is removed if it exists inside unquoted fields
            gsub(/\r/, "", val)
            printf "%s%s", val, (i == NF ? "" : OFS)
        }
        print ""
    }
    END {
        # Ensure output ends with a newline if file was not empty
        if (NR > 0) {
            # awk print already adds newline
        }
    }
    ' "$1"
}