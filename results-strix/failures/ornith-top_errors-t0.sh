top_errors() {
    local logfile="$1" n="$2"
    [ -z "$n" ] && n=10
    [ ! -f "$logfile" ] && return 0

    awk '
        /^ERROR: / {
            sig = substr($0, 8)
            gsub(/[0-9]+/, "#", sig)
            count[sig]++
        }
        END {
            for (s in count) {
                printf "%d\t%s\n", count[s], s
            }
        }
    ' "$logfile" \
    | sort -k1,1nr -k2,2 \
    | head -n "$n" \
    | awk -F'\t' '{ printf "%s %s\n", $1, $2 }'
}