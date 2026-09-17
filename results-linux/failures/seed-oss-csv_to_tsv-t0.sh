csv_to_tsv() {
    awk '
BEGIN {
    inside_quotes = 0
    record_length = 0
}

{
    line = $0
    sub(/\r$/, "", line)

    len = length(line)
    for (i = 1; i <= len; i++) {
        c = substr(line, i, 1)
        if (c == "\"") {
            if (i < len && substr(line, i+1, 1) == "\"") {
                record[record_length] = record[record_length] "\""
                i++
            } else {
                inside_quotes = 1 - inside_quotes
            }
        } else if (inside_quotes == 0) {
            if (c == ",") {
                record_length++
            } else if (c == "\n") {
                process_record()
                record_length = 0
                delete record
            } else {
                record[record_length] = record[record_length] c
            }
        } else {
            if (c == "\n") {
                record[record_length] = record[record_length] "\\n"
            } else if (c == "\t") {
                record[record_length] = record[record_length] "\\t"
            } else {
                record[record_length] = record[record_length] c
            }
        }
    }
}

END {
    if (record_length >= 0) {
        process_record()
    }
}

function process_record() {
    output = record[0]
    for (i = 1; i <= record_length; i++) {
        output = output "\t" record[i]
    }
    print output
    record_length = 0
    delete record
}' "$1"
}