```bash
csv_col() {
    local name="$1"
    awk -v col="$name" '
    function parse_csv(line, fields,    n, i, c, field, in_quotes) {
        n = 0; field = ""; in_quotes = 0
        for (i = 1; i <= length(line); i++) {
            c = substr(line, i, 1)
            if (in_quotes) {
                if (c == "\"") {
                    if (i < length(line) && substr(line, i+1, 1) == "\"") {
                        field = field "\""; i++
                    } else { in_quotes = 0 }
                } else { field = field c }
            } else {
                if (c == "\"") { in_quotes = 1 }
                else if (c == ",") { fields[++n] = field; field = "" }
                else { field = field c }
            }
        }