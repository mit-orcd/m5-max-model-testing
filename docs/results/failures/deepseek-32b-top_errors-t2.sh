top_errors() {
    grep 'ERROR: ' "$1" |
        sed -e 's/.*ERROR: //' -e 's/[0-9]+/#/g' |
        sort |
        uniq -c |
        sed -e 's/^\s*\([0-9]+\)\s+/\1 /' |
        sort -k1nr -k2d |
        head -n "$2"
}