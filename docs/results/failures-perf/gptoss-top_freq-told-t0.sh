top_freq() {
    local file="$1"
    sort "$file" | uniq -c | awk '{c=$1; $1=""; sub(/^ +/,""); print c, $0}' | sort -nr | head -n 10
}