top_errors() {
    local logfile=$1
    local n=$2
    awk '
    BEGIN { OFS="\t" }
    {
        if (index($0, "ERROR: ") > 0) {
            sig = substr($0, index($0, "ERROR: ") + 7)
            gsub(/[0-9]+/, "#", sig)
            counts[sig]++
        }
    }
    END {
        for (s in counts) {
            print counts[s], s
        }
    }' "$logfile" | sort -t$'\t' -k1,1nr -k2,2 | head -n "$n"
}