top_freq() {
  sort "$1" | uniq -c | sort -nr | head -10 | awk '{print $1, $2}'
}