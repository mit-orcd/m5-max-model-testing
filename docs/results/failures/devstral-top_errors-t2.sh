top_errors() {
    local logfile="$1"
    local n="$2"
    awk '
        /ERROR: / {
            sig = substr($0, index($0, "ERROR: ") + 7)
            sub(/[ \t]+$/, "", sig)
            for (i = 1; i <= length(sig); i++) {
                if (sig ~ /^[0-9]/) {
                    sub(/[0-9]+/, "#", sig)
                } else {
                    break
                }
            }
            seen[sig]++
        }
        END {
            for (sig in seen) {
                print seen[sig], sig
            }
        }
    ' "$logfile" | sort -k1,1nr -k2,2 | head -n "$n"
}