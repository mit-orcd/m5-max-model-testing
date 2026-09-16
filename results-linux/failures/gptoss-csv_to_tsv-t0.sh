csv_to_tsv() {
    local file="$1"
    awk '
    BEGIN {
        RS = "\r?\n"
        ORS = "\n"
        balance = 0
        record = ""
    }
    {
        line = $0
        sub(/\r$/, "", line)          # strip CR
        # Count unescaped quotes
        tmp = line
        gsub(/""/, "__DQ__", tmp)     # placeholder for escaped quotes
        n = split(tmp, a, "\"")
        q = n - 1
        balance = (balance + q) % 2
        record = record line "\n"
        if (balance == 0) {
            process(record)
            record = ""
        }
    }
    END {
        if (record != "") {
            process(record)
        }
    }
    function process(rec,   i, len, out, field, pos, c, i2, ch, inq) {
        sub(/\n$/, "", rec)          # remove trailing newline
        out = ""
        pos = 1
        len = length(rec)
        while (pos <= len) {
            if (substr(rec, pos, 1) == "\"") {
                # quoted field
                pos++
                field = ""
                inq = 1
                while (pos <= len && inq) {
                    ch = substr(rec, pos, 1)
                    if (ch == "\"") {
                        if (pos < len && substr(rec, pos+1, 1) == "\"") {
                            field = field "\""
                            pos += 2
                        } else {
                            pos++
                            inq = 0
                        }
                    } else {
                        field = field ch
                        pos++
                    }
                }
                # skip comma if present
                if (pos <= len && substr(rec, pos, 1) == ",") pos++
            } else {
                # unquoted field
                field = ""
                while (pos <= len && substr(rec, pos, 1) != ",") {
                    field = field substr(rec, pos, 1)
                    pos++
                }
                if (pos <= len && substr(rec, pos, 1) == ",") pos++
            }
            # restore escaped quotes
            gsub(/__DQ__/, "\"", field)
            # escape tabs and newlines
            gsub(/\t/, "\\t", field)
            gsub(/\n/, "\\n", field)
            out = out field "\t"
        }
        sub(/\t$/, "", out)
        print out
    }
    ' "$file"
}