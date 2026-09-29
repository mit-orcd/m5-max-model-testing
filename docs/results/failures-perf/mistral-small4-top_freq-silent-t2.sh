top_freq() {
  awk '{count[$1]++} END {for (i in count) printf "%d %s\n", count[i], i}' "$1" | sort -rn | head -n 10
}