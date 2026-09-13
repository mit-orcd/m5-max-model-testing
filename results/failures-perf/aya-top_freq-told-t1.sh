top_freq() {
  local file="$1"
  local tmpfile=$(mktemp)
  local cmd="sort -n -r -t$'\t' -k1,1 -s -o $tmpfile"
  local time=$(date +%s)
  cat "$file" |
    awk -F'\t' '
      $1 in a[$1] { a[$1]++ }
      END { for (i in a) print a[i], i | "'$cmd'" }
    ' |
    awk -F'\t' '{$1=$2$1;print}' |
    head -n 10 |
    tr -s ' ' '\t' |
    sed 's/\t/\ /g'
  rm "$tmpfile"
  echo "Time taken: $(date +%s -d@$((`date +%s` - $time)))s"
}