top_freq() {
    sort "$1" | uniq -c | sort -rn | head -n 10 | awk '{ $1=""; sub(/^ /, ""); print $0 }' | awk '{ match($0, /^ /); print $0 }' | sed 's/^ *//'
}