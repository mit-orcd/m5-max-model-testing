csv_to_tsv() {
    awk '
    BEGIN {
        buffer = ""
        printed = 0
    }

    {
        sub(/\r$/, "", $0)
        buffer = buffer $0

        temp = gensub(/""/, "\0", "g", buffer)
        total_quotes = gsub(/"/, "", temp)
        if (total_quotes % 2 == 0) {
            process_record(buffer)
            printed = 1
            buffer = ""
        }
    }

    END {
        if (buffer != "") {
            process_record(buffer)
            printed = 1
        }
        if (printed == 0) {
            print ""
        }
    }

    function process_record(buffer,    field_count, current_field, in_quotes, i, c, len, fields) {
        field_count = 0
        current_field = ""
        in_quotes = 0
        len = length(buffer)
        i = 1
        while (i <= len) {
            c = substr(buffer, i, 1)
            if (c == "\"") {
                if (i + 1 <= len && substr(buffer, i + 1, 1) == "\"") {
                    current_field = current_field "\""
                    i = i + 2
                } else {
                    in_quotes = 1 - in_quotes
                    i = i + 1
                }
            } else if (c == "," && !in_quotes) {
                field_count = field_count + 1
                fields[field_count] = current_field
                current_field = ""
                i = i + 1
            } else {
                current_field = current_field c
                i = i + 1
            }
        }
        field_count = field_count + 1
        fields[field_count] = current_field

        processed_fields = ""
        for (f = 1; f <= field_count; f++) {
            field = fields[f]
            if (field ~ /^".*"$/) {
                field = substr(field, 2, length(field) - 2)
                gsub(/""/, "\"", field)
            }
            gsub(/\t/, "\\t", field)
            gsub(/\n/, "\\n", field)
            if (f == 1) {
                processed_fields = field
            } else {
                processed_fields = processed_fields "\t" field
            }
        }
        print processed_fields
    }
    ' "$1"
}