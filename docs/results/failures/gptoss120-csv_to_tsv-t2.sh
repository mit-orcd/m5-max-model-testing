csv_to_tsv() {
    local src="${1:-/dev/stdin}"
    awk '
    {
        data = data $0 "\n"
    }
    END {
        n = length(data)
        field = ""
        inquote = 0
        flen = 0
        for (i = 1; i <= n; i++) {
            c = substr(data, i, 1)
            if (inquote) {
                if (c == "\"") {
                    nxt = (i < n) ? substr(data, i+1, 1) : ""
                    if (nxt == "\"") {
                        field = field "\""
                        i++
                    } else {
                        inquote = 0
                    }
                } else if (c == "\n") {
                    field = field "\\n"
                } else if (c == "\t") {
                    field = field "\\t"
                } else {
                    field = field c
                }
            } else {
                if (c == "\"") {
                    inquote = 1
                } else if (c == ",") {
                    outfield()
                } else if (c == "\n") {
                    outfield()
                    outrecord()
                } else if (c == "\r") {
                    # ignore CR
                } else {
                    field = field c
                }
            }
        }
        # last record without trailing newline
        if (field != "" || flen > 0) {
            outfield()
            outrecord()
        }

        function outfield(   ) {
            fields[flen++] = field
            field = ""
        }
        function outrecord(   i) {
            for (i = 0; i < flen; i++) {
                printf "%s", fields[i]
                if (i < flen-1) printf "\t"
            }
            printf "\n"
            delete fields
            flen = 0
        }
    }' "$src"
}