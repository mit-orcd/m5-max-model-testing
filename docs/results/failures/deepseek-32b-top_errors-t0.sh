top_errors() {
    grep 'ERROR: ' "$1" | cut -d 'ERROR: ' -f2 | sed 's/[0-9]\+/#/g' | sort | uniq -c | sort -k1,1nr -k2,2 | head -n "$2"
}