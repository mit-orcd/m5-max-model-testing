largest_file() {
  local dir="$1"
  
  # Handle empty input
  [ -z "$dir" ] && { echo "Usage: largest_file <dir>" >&2; return 2; }
  
  # Check directory exists
  [ -d "$dir" ] || { echo "Error: '$dir' is not a directory" >&2; return 1; }
  
  local largest_size=-1
  local largest_file=""
  
  # Find all regular files, compute their sizes, and track the largest
  while IFS=$'\t' read -r size path; do
    if [ "$size" -gt "$largest_size" ]; then
      largest_size="$size"
      largest_file="$path"
    fi
  done < <(
    find "$dir" -type f -printf '%s\t%p\n' 2>/dev/null | sort -rn
  )
  
  # Output the largest file path (if any found)
  if [ -n "$largest_file" ]; then
    printf '%s\n' "$largest_file"
  else
    echo "Error: No regular files found under '$dir'" >&2
    return 1
  fi
}