csv_to_tsv() {
    local file="$1"
    awk '
    BEGIN {
        RS = "\n"
        ORS = "\n"
        FS = ""
    }
    {
        # Normalize line ending: remove trailing CR if present
        gsub(/\r$/, "")

        # Process the line character by character to handle RFC-4180
        len = length($0)
        field = ""
        in_quotes = 0
        record = ""
        i = 1
        while (i <= len) {
            c = substr($0, i, 1)
            if (in_quotes) {
                if (c == "\"") {
                    # Check for escaped quote ""
                    if (i < len && substr($0, i+1, 1) == "\"") {
                        field = field "\""
                        i++
                    } else {
                        in_quotes = 0
                    }
                } else {
                    field = field c
                }
            } else {
                if (c == "\"") {
                    in_quotes = 1
                } else if (c == ",") {
                    # Escape internal tabs and newlines for TSV output
                    gsub(/\t/, "\\t", field)
                    gsub(/\n/, "\\n", field)
                    if (record == "") {
                        record = field
                    } else {
                        record = record "\t" field
                    }
                    field = ""
                } else {
                    field = field c
                }
            }
            i++
        }
        # End of record
        gsub(/\t/, "\\t", field)
        gsub(/\n/, "\\n", field)
        if (record == "") {
            record = field
        } else {
            record = record "\t" field
        }
        print record
    }
    ' "$file"
}