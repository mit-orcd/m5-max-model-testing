csv_to_tsv() {
    local file=${1:-/dev/stdin}
    awk '
    function parse(rec,    i,c,field,inside,fields,n,out,f) {
        field=""; inside=0; n=0
        for (i=1; i<=length(rec); i++) {
            c = substr(rec,i,1)
            if (c == "\"") {
                if (inside && substr(rec,i+1,1) == "\"") {
                    field = field "\""
                    i++
                } else {
                    inside = !inside
                }
            } else if (c == "," && !inside) {
                fields[++n] = field
                field = ""
            } else if (c == "\r") {
                # ignore CR
            } else {
                field = field c
            }
        }
        fields[++n] = field
        out = ""
        for (i=1; i<=n; i++) {
            f = fields[i]
            gsub(/\t/, "\\\\t", f)
            gsub(/\n/, "\\\\n", f)
            out = out (i==1 ? "" : "\t") f
        }
        print out
    }
    {
        # strip trailing CR from the input line
        sub(/\r$/, "", $0)
        line = $0

        # accumulate lines that belong to the same CSV record
        rec = (rec == "" ? line : rec "\n" line)

        # update quote‑balance state (ignoring escaped double quotes)
        for (i=1; i<=length(line); i++) {
            c = substr(line,i,1)
            if (c == "\"") {
                if (i < length(line) && substr(line,i+1,1) == "\"") {
                    i++                     # skip escaped quote
                } else {
                    inquote = !inquote      # toggle quoted state
                }
            }
        }

        # when not inside a quoted field we have a complete record
        if (!inquote) {
            parse(rec)
            rec = ""
        }
    }
    END {
        if (rec != "")
            parse(rec)
    }
    ' "$file"
}