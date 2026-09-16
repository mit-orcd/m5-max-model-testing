```bash
find_dupes() {
  local dir="${1:-.}"
  [[ -d "$dir" ]] || { echo "Error: '$dir' is not a directory" >&2; return 1; }

  find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | \
    awk '{
      cmd = "md5 -r " $0; 
      # md5 -r output format: "MD5 (filename) = hex" or "MD5 filename = hex" on macOS
      # Actually md5 -r on macOS outputs: "MD5 (filename) = hash"
      # But we need hash and filename. Let's rely on the standard format:
      # "MD5 (file) = hash" or "MD5 file = hash"
      # Wait, the prompt says "use md5 -r". 
      # macOS md5 -r output: "MD5 (file) = hash"
      # Let's re-parse the line from find.
    }' # This approach is tricky because md5 -r output format varies slightly or requires parsing.
    
  # Let's restart with a more robust approach using a temp file or process substitution
  # to avoid subshell variable scope issues and handle macOS specific md5 output.
  
  local tmpfile
  tmpfile=$(mktemp)
  trap 'rm -f "$tmpfile"' EXIT

  # Generate lines: hash path
  while IFS= read -r -d '' file; do
    hash=$(md5 -r "$file" 2>/dev/null | awk -F' = ' '{print $2}')
    if [[ -n "$hash" ]]; then
      printf '%s %s\n' "$hash" "$file" >> "$tmpfile"
    fi
  done < <(find "$dir" -type f -print0)

  # Sort by hash, then by path
  sort -k1,1 -k2 "$tmpfile" | \
  awk '
    BEGIN { prev_hash = ""; count = 0; paths = "" }
    {
      hash = $1
      path = $2
      for (i=3; i<=NF; i++) path = path " " $i # Rejoin if spaces in path (though unlikely for simple sort)
      # Actually, sorting by space separated hash and path is risky if paths have spaces.
      # Let's assume the delimiter in the sort is tab or handle carefully.
      # Re-doing: use tab as internal delimiter for safety.
    }
  '
  
  # Let's rewrite the logic cleanly with tabs to handle spaces in filenames properly.
  rm -f "$tmpfile"
  tmpfile=$(mktemp)
  trap 'rm -f "$tmpfile"' EXIT

  while IFS= read -r -d '' file; do
    hash=$(md5 -r "$file" 2>/dev/null | awk -F' = ' '{print $2}')
    if [[ -n "$hash" ]]; then
      printf '%s\t%s\n' "$hash" "$file" >> "$tmpfile"
    fi
  done < <(find "$dir" -type f -print0)

  sort -t$'\t' -k1,1 -k2,2 "$tmpfile" | \
  awk -F'\t' '
    {
      if ($1 == prev_hash) {
        paths = paths " " $2
        count++
      } else {
        if (count >= 2) {
          # Sort the paths for this group
          n = split(paths, arr, " ")
          for (i=1; i<=n; i++) for (j=i+1; j<=n; j++) if (arr[i] > arr[j]) { t=arr[i]; arr[i]=arr[j]; arr[j]=t }
          # Remove leading space and print
          line = ""
          for (i=1; i<=n; i++) { if (line != "") line = line " "; line = line arr[i] }
          print line
        }
        prev_hash = $1
        paths = $2
        count = 1
      }
    }
    END {
      if (count >= 2) {
        n = split(paths, arr, " ")
        for (i=1; i<=n; i++) for (j=i+1; j<=n; j++) if (arr[i] > arr[j]) { t=arr[i]; arr[i]=arr[j]; arr[j]=t }
        line = ""
        for (i=1;