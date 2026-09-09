csv_col() {
    local name="$1"
    awk -v colname="$name" '
        BEGIN {
            # Match CSV fields, handling quoted fields with commas
            FPAT = "([^,]+)|(\"([^\"]|\"\")*\")"
        }
        NR == 1 {
            # Find the column index for the requested header name
            for (i = 1; i <= NF; ++i) {
                h = $i
                if (h ~ /^".*"$/) {
                    gsub(/^"|"$/,"",h)
                    gsub(/""/,"\"",h)
                }
                if (h == colname) {
                    col = i
                    break
                }
            }
            if (col == 0) exit 1      # column not found
            next
        }
        {
            field = $col
            if (field ~ /^".*"$/) {
                gsub(/^"|"$/,"",field)
                gsub(/""/,"\"",field)
            }
            print field
        }
    '
}