csv_to_tsv() {
    local file="$1"
    local content
    content=$(cat "$file" | tr -d '\r')
    awk '
    BEGIN {
        FS = ""
        OFS = ""
        in_quotes = 0
        field = ""
        first_field = 1
        first_record = 1
    }
    {
        for (i = 1; i <= NF; i++) {
            c = $i
            if (in_quotes) {
                if (c == "\"") {
                    if (i < NF && $(i+1) == "\"") {
                        field = field "\""
                        i++
                    } else {
                        in_quotes = 0
                    }
                } else {
                    if (c == "\n") {
                        field = field "\\n"
                    } else if (c == "\t") {
                        field = field "\\t"
                    } else {
                        field = field c
                    }
                }
            } else {
                if (c == "\"") {
                    in_quotes = 1
                } else if (c == ",") {
                    if (!first_field) printf "\t"
                    printf "%s", field
                    field = ""
                    first_field = 0
                } else if (c == "\n") {
                    if (!first_field) printf "\t"
                    printf "%s", field
                    field = ""
                    first_field = 1
                    if (!first_record) printf "\n"
                    first_record = 0
                } else {
                    if (c == "\t") {
                        field = field "\\t"
                    } else {
                        field = field c
                    }
                }
            }
        }
        if (in_quotes) {
            field = field "\\n"
        }
    }
    END {
        if (field != "" || !first_record) {
            if (!first_record) printf "\t"
            printf "%s", field
            printf "\n"
        }
    }
    ' "$file"
}