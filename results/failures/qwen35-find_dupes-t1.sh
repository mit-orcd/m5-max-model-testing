find_dupes() {
  local dir="${1:-.}"
  
  [ -d "$dir" ] || return 1

  # Collect MD5 hashes and paths for regular files
  # Using BSD 'md5 -r' format: <hash>  <path>
  # We need to handle spaces in filenames correctly
  local temp_file
  temp_file=$(mktemp)
  trap "rm -f '$temp_file'" EXIT

  find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
    # Get md5 hash using BSD md5
    hash=$(md5 -r "$file" 2>/dev/null | awk '{print $1}')
    if [ -n "$hash" ]; then
      # Store hash and path separated by a null byte to handle spaces/special chars
      printf '%s\t%s\0' "$hash" "$file"
    fi
  done > "$temp_file"

  # Process the collected data
  # Sort by hash, then by path
  # Use a custom sort key: hash (field 1) then path (field 2)
  # Since we used tabs as delimiters, we can sort by hash, then path
  
  # Re-read and group
  declare -A groups
  
  while IFS=$'\t' read -r hash path; do
    if [ -n "$hash" ]; then
      if [ -z "${groups[$hash]}" ]; then
        groups[$hash]="$path"
      else
        groups[$hash]="${groups[$hash]}"$'\n'"$path"
      fi
    fi
  done < "$temp_file"

  # Iterate over hashes with multiple files
  for hash in "${!groups[@]}"; do
    files="${groups[$hash]}"
    count=$(echo "$files" | wc -l)
    
    if [ "$count" -ge 2 ]; then
      # Sort the paths within the group and join with space
      sorted_paths=$(echo "$files" | sort | tr '\n' ' ' | sed 's/ $//')
      # We need to store this group with its first path for final sorting
      first_path=$(echo "$files" | sort | head -n 1)
      printf '%s\t%s\n' "$first_path" "$sorted_paths"
    fi
  done | sort -t$'\t' -k1,1 | cut -f2
}