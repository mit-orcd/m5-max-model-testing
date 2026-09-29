csv_col() {
    local col="$1"
    gawk -v col="$col" '
        BEGIN {
            # FPAT matches a CSV field: either quoted (allowing doubled quotes) or unquoted
            FPAT = "([^,]*|\"([^\"]|\"\")*\")*"
        }
        NR == 1 {
            # Find the column index from the header line
            for (i = 1; i <= NF; i++) {
                hdr = $i
                if (hdr ~ /^".*"$/) {
                    gsub(/^"/, "", hdr)
                    gsub(/"$/, "", hdr)
                    gsub(/""/, "\"", hdr)
                }
                if (hdr == col) {
                    idx = i
                    break
                }
            }
            if (!idx) exit 1   # column not found
            next
        }
        {
            if (idx > NF) next
            val = $idx
            # Strip surrounding quotes and un‑escape doubled quotes
            if (val ~ /^".*"$/) {
                gsub(/^"/, "", val)
                gsub(/"$/, "", val)
                gsub(/""/, "\"", val)
            }
            print val
        }
    '
}