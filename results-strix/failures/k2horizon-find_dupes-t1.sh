find_dupes() {
  local dir="$1"
  [ -d "$dir" ] || return 1
  local -A groups=()
  while IFS= read -r file; do
    local md5
    md5=$(md5 -q "$file" 2>/dev/null) || continue
    groups["$md5"]="$groups[$md5] $file"
  done < <(find "$dir" -type f -print 2>/dev/null)
  local -a lines=()
  local key
  for key in "${!groups[@]}"; do
    local line="${groups[$key]# }"
    if [ "$(echo "$line" | wc -l)" -ge 2 ]; then
      lines+=("$line")
    fi
  done
  printf '%s\n' "${lines[@]}" | sort | awk '
    BEGIN { RS=""; OFS="\n" }
    {
      n = split($0, paths, " ")
      for (i = 1; i <= n; i++) sort(paths, i)
      for (i = 1; i <= n; i++) printf "%s%s", paths[i], (i < n ? " " : "\n")
    }'
}