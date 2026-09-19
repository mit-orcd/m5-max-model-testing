csv_to_tsv() {
    local file="$1"
    awk '
    BEGIN {
        FS = ""
        OFS = ""
        RS = "\r?\n"
    }
    {
        line = $0
        n = length(line)
        out = ""
        in_quote = 0
        field = ""
        i = 1
        while (i <= n) {
            c = substr(line, i, 1)
            if (!in_quote) {
                if (c == "\"") {
                    in_quote = 1
                } else if (c == ",") {
                    print_field(field)
                    field = ""
                } else {
                    field = field c
                }
            } else {
                if (c == "\"") {
                    if (substr(line, i+1, 1) == "\"") {
                        field = field "\""
                        i++
                    } else {
                        in_quote = 0
                    }
                } else if (c == "\t") {
                    field = field "\\t"
                } else if (c == "\n") {
                    field = field "\\n"
                } else {
                    field = field c
                }
            }
            i++
        }
        print_field(field)
        print ""
    }
    function print_field(f) {
        if (out != "") {
            out = out "\t"
        }
        out = out f
    }
    END {
        if (out != "") {
            print out
        }
    }
    ' "$file"
}