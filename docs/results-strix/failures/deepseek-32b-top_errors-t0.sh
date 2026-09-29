top_errors() {
    grep 'ERROR: ' "$1" | sed 's/.*ERROR: //; s/[0-9]\+/#/g' | sort | uniq -c | sort -k1,1nr -k2,2 | head -n "$2"
}