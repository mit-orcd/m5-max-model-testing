total_size() {
  local dir=${1:-.}
  local sum=0
  while IFS= read -r -d '' file; do
    local sz
    sz=$(stat -f%z "$file" 2>/dev/null)
    sum=$((sum + sz))
  done < <(find -- "$dir" -type f -print0)
  printf '%d\n' "$sum"
}