top_errors() {
    local logfile="$1"
    local n="$2"
    [ -z "$logfile" ] || [ -z "$n" ] && return
    awk '
    /ERROR: / {
        sig = substr($0, RSTART + RLENGTH)
        sub(/^ +/, "", sig)
        gsub(/[0-9]+/, "#", sig)
        count[sig]++
    }
    END {
        for (s in count) {
            print count[s], s
        }
    }
    ' "$logfile" | sort -k1,1nr -k2,2 | head -n "$n"
}