csv_to_tsv() {
    # Use AWK to handle the complex RFC-4180 parsing logic.
    # We use a state machine to parse fields according to the spec.
    awk '
    BEGIN {
        FPAT = "([^,\"]*)|(\"([^\"]|\"\")*\")"
        OFS = "\t"
    }
    {
        for (i = 1; i <= NF; i++) {
            field = $i
            # Check if field is quoted
            if (field ~ /^".*"$/) {
                # Remove surrounding quotes
                field = substr(field, 2, length(field) - 2)
                # Handle escaped double quotes "" -> "
                gsub(/""/, "\"", field)
                # Handle internal tabs and newlines as per requirement
                gsub(/\t/, "\\t", field)
                gsub(/\n/, "\\n", field)
                gsub(/\r/, "", field)
            } else {
                # For unquoted fields, just strip potential \r from CRLF
                gsub(/\r/, "", field)
            }
            printf "%s%s", field, (i == NF ? "" : OFS)
        }
        print ""
    }
    ' "$1"
}