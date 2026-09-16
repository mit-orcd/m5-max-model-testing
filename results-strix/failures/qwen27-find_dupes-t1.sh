find_dupes() {
  local dir="$1"
  # Find all regular files, compute md5, group by hash
  # Using md5 -r (BSD) which outputs "hash filename"
  local tmpfile
  tmpfile=$(mktemp)
  
  # Get all regular files recursively
  find "$dir" -type f -print0 | while IFS= read -r -d '' f; do
    md5 -r "$f"
  done | sort > "$tmpfile"
  
  # Group by first field (md5 hash), collect paths
  local prev_hash=""
  local group_paths=""
  
  while IFS= read -r line; do
    local hash="${line%% *}"
    local path="${line#* }"
    
    if [[ -n "$prev_hash" && "$prev_hash" != "$hash" ]]; then
      # Process previous group if it has 2+ files
      local count
      count=$(echo "$group_paths" | wc -w | tr -d ' ')
      if [[ "$count" -ge 2 ]]; then
        # Sort paths within group and output
        echo "$group_paths" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//'
      fi
      group_paths=""
    fi
    
    prev_hash="$hash"
    group_paths="$group_paths $path"
  done < "$tmpfile"
  
  # Process last group
  if [[ -n "$prev_hash" && -n "$group_paths" ]]; then
    local count
    count=$(echo "$group_paths" | wc -w | tr -d ' ')
    if [[ "$count" -ge 2 ]]; then
      echo "$group_paths" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//'
    fi
  fi
  
  # Collect all groups, sort by first path, and print
  rm -f "$tmpfile"
}