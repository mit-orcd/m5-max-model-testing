csv_to_tsv() {
    tr -d '\r' < "$1" | awk -v FPAT='([^,]+)|("[^"]*")' '
        {
            for (i=1; i<=NF; i++) {
                if ($i ~ /^".*"$/) {
                    $i = substr($i, 2, length($i)-2);
                    gsub(/""/, "\"", $i);
                }
                gsub(/\t/, "\\t", $i);
                gsub(/\n/, "\\n", $i);
                fields[i] = $i;
            }
            line = "";
            for (i=1; i<=length(fields); i++) {
                if (i > 1) {
                    line = line "\t";
                }
                line = line fields[i];
            }
            print line;
        }
    '
}