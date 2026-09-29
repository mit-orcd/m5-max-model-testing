top_freq() {
  local file="$1"
  awk '{count[$1]++} END {for (key in count) printf "%d %s\n", count[key], key}' "$file" | sort -rn | head -n 10
}