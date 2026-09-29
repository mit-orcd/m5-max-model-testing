csv_col() {
  local col="$1"
  awk -v target="$col" '
    NR == 1 {
      for (i = 1; i <= NF; i++) {
        if ($i == target) { idx = i; break }
      }
    }
    NR > 1 { print $idx }
  '
}