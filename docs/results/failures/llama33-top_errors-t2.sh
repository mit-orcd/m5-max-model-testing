top_errors() {
  grep 'ERROR:' "$1" | 
  sed 's/[0-9]\+/#/g' | 
  cut -d' ' -f2- | 
  sort | 
  uniq -c | 
  sort -rn -k1,1 -k2 | 
  head -n "$2"
}