csv_col() {
    local colname="$1"
    awk -v colname="$colname" '
        BEGIN {
            # CSV field pattern: unquoted fields or quoted fields (allowing commas inside)
            FPAT = "([^,]*|\"([^\"]|\"\")*\")"
        }
        NR == 1 {
            for (i = 1; i <= NF; i++) {
                # Strip surrounding quotes for header comparison
                hdr = $i
                gsub(/^\"|\"$/,"",hdr)
                if (hdr == colname) {
                    col = i
                    break
                }
            }
            if (!col) exit 1   # column not found
            next
        }
        {
            field = $col
            gsub(/^\"|\"$/,"",field)   # remove surrounding quotes from data
            print field
        }
    '
}