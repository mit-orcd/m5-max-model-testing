function top_errors {
    grep '^ERROR: ' "$1" | sed -e 's/^ERROR: //' -e 's/[0-9]\+/#/g' | sort | uniq -c | sort -k1nr -k2 | head -n "$2"
}