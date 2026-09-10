csv_to_tsv() {
    local file="${1:-/dev/stdin}"
    awk '
    # Escape tabs and newlines inside a field
    function esc(s,   t) {
        gsub(/\t/, "\\\\t", s)
        gsub(/\n/, "\\\\n", s)
        return s
    }

    # Parse a complete CSV record stored in "rec"
    function parse(rec,   i, n, c, nxt, inq, fld) {
        n = length(rec)
        fld = ""
        inq = 0
        nf = 0
        for (i = 1; i <= n; i++) {
            c = substr(rec, i, 1)
            if (inq) {
                if (c == "\"") {
                    nxt = substr(rec, i+1, 1)
                    if (nxt == "\"") {
                        fld = fld "\""
                        i++
                    } else {
                        inq = 0
                    }
                } else {
                    fld = fld c
                }
            } else {
                if (c == ",") {
                    fields[++nf] = esc(fld)
                    fld = ""
                } else if (c == "\"") {
                    inq = 1
                } else {
                    fld = fld c
                }
            }
        }
        fields[++nf] = esc(fld)
    }

    # Output the fields of the current record as TSV
    function output(   i, line) {
        line = fields[1]
        for (i = 2; i <= nf; i++) line = line "\t" fields[i]
        print line
        delete fields
        nf = 0
    }

    {
        sub(/\r$/, "")                 # strip CR from CRLF endings
        line = $0
        buf = (buf == "" ? line : buf "\n" line)   # accumulate lines

        # count total double‑quotes in the buffer
        qcnt = gsub(/"/, "&", buf)

        if (qcnt % 2 == 0) {           # even → complete record
            parse(buf)
            output()
            buf = ""
        }
    }
    END {
        if (buf != "") {               # last record may lack trailing NL
            parse(buf)
            output()
        }
    }
    ' "$file"
}