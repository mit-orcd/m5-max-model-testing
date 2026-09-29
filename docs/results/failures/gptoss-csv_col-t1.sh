csv_col() {
  local col="$1"
  awk -v col="$col" '
    NR==1 {
      for (i=1; i<=NF; i++) {
        if ($i == col) { idx = i; break }
      }
      if (!idx) exit 1
    }
    NR>1 && idx { print $idx }
  '
}