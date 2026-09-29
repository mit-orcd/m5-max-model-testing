rotate() {
  local file="$1"
  local new_file="${file}.1"
  if [ -f "$new_file" ]; then
    local next_file="${file}.$(($(ls "$file".* 2>/dev/null | cut -d. -f3 | sort -n | tail -1) + 1))"
    if [ -n "$next_file" ]; then
      mv "$new_file" "$next_file"
    fi
  fi
  mv "$file" "$new_file" 2>/dev/null
}