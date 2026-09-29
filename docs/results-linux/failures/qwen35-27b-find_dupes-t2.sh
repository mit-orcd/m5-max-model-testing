find_dupes() {
  local dir="$1"
  local -a files
  local -A md5_map
  
  # Find all regular files and compute MD5
  while IFS= read -r -d '' file; do
    local hash
    hash=$(md5 -q "$file" 2>/dev/null) || continue
    files+=("$file")
    md5_map["$hash"]+="$file"$'\n'
  done < <(find "$dir" -type f -print0 2>/dev/null)
  
  # Process groups
  for hash in "${!md5_map[@]}"; do
    local -a group
    local -a sorted_group
    local count=0
    
    # Read files for this hash
    while IFS= read -r file; do
      [[ -n "$file" ]] && group+=("$file")
    done <<< "${md5_map[$hash]}"
    
    # Skip if only one file
    count=${#group[@]}
    (( count < 2 )) && continue
    
    # Sort files within group
    mapfile -t sorted_group < <(printf '%s\n' "${group[@]}" | sort)
    
    # Print space-separated sorted paths
    printf '%s ' "${sorted_group[@]}" | sed 's/ $/\n/'
  done | sort
}