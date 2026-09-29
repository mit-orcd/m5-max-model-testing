csv_to_tsv() {
    awk -v RS='\\' '
    function unesc(s,   out, i, c) {
        out = ""; i = 1
        while (i <= length(s)) {
            c = substr(s, i, 1)
            if (c == "\\") {
                i++
                c = substr(s, i, 1)
                if (c == "n") out = out "\n"
                else if (c == "t") out = out "\t"
            }
            out = out c; i++
        }
        return out
    }
    function csv_field(s,   out, i, inq, esc, c) {
        out = ""; i = 1; inq = 0; esc = 0
        while (i <= length(s)) {
            c = substr(s, i, 1)
            if (esc) { out = out c; esc = 0; i++; continue }
            if (inq) {
                if (c == "\"") {
                    if (i < length(s) && substr(s, i+1, 1) == "\"") { out = out "\""; i += 2; continue }
                    inq = 0; i++; continue
                }
                out = out c; i++; continue
            }
            if (c == "\"") { inq = 1; i++; continue }
            if (c == ",") break
            out = out c; i++
        }
        return out
    }
    {
        line = $0
        if (line ~ /\\r$/) line = substr(line, 1, length(line)-1)
        fields = ""; i = 1; n = split(line, parts, ",")
        for (j = 1; j <= n; j++) {
            f = csv_field(parts[j])
            fields = fields (j > 1 ? "\t" : "") f
        }
        gsub(/\n/, "\\n", fields)
        gsub(/\t/, "\\t", fields)
        print fields
    }
    END { if (NR == 0) print "" }
' "$1"
}