csv_to_tsv() {
    awk '
    function emit_field() {
        gsub(/\t/, "\\t", field)
        gsub(/\n/, "\\n", field)
        line = (nf ? line "\t" : "") field
        nf++
        field = ""
    }
    function emit_line() {
        emit_field()
        print line
        line = ""; nf = 0
    }
    {
        data = $0 "\n"   # RS re-adds the newline; last line may lack one, fixed in END
        for (i = 1; i <= length(data); i++) {
            c = substr(data, i, 1)
            if (inq) {
                if (c == "\"") {
                    if (substr(data, i + 1, 1) == "\"") { field = field "\""; i++ }
                    else inq = 0
                } else if (c == "\r" && substr(data, i + 1, 1) == "\n") {
                    # CRLF inside quotes: keep the \n, drop the \r
                } else field = field c
            } else if (c == "\"") inq = 1
            else if (c == ",") emit_field()
            else if (c == "\n") emit_line()
            else if (c == "\r") { }
            else field = field c
        }
    }
    END {
        if (inq || field != "" || nf) emit_line()
    }
    ' "$1"
}
