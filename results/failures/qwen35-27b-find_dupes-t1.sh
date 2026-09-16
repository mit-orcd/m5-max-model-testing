find_dupes() {
  local dir="$1"
  [[ -z "$dir" ]] && return 1

  # Create a temporary file to store checksums and paths
  local tmpfile
  tmpfile=$(mktemp)
  trap "rm -f '$tmpfile'" EXIT

  # Find all regular files and compute MD5
  # Using BSD find and md5 -r (output format: "MD5  filename")
  find "$dir" -type f -exec md5 -r {} + | while IFS='  ' read -r checksum path; do
    # Normalize path to handle potential spaces or special chars if needed, 
    # though md5 -r output usually handles this well with the space separator.
    # BSD md5 -r outputs: "<hash>  <filename>" (two spaces)
    echo "$checksum $path"
  done | sort -t' ' -k2 > "$tmpfile"

  # Process the sorted list to find duplicates
  local prev_hash=""
  local current_group=""
  local group_count=0

  while IFS=' ' read -r hash filepath; do
    if [[ "$hash" == "$prev_hash" ]]; then
      current_group="$current_group $filepath"
      ((group_count++))
    else
      # Output previous group if it had duplicates
      if ((group_count >= 2)); then
        # Sort paths within the group
        echo "$current_group" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//'
        echo
      fi
      
      # Start new group
      current_group="$filepath"
      prev_hash="$hash"
      group_count=1
    fi
  done < "$tmpfile"

  # Handle the last group
  if ((group_count >= 2)); then
    echo "$current_group" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//'
    echo
  fi
}