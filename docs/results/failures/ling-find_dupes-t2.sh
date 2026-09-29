find_dupes() {
  (
    cd "$1" 2>/dev/null || return
    md5_files=$(find . -type f -exec md5 -r {} + 2>/dev/null | sort -k1,1)
    current_md5=""
    group=""
    while IFS= read -r line; do
      md5_val=$(echo "$line" | awk '{print $1}')
      file_path=$(echo "$line" | awk '{print $2}')
      if [ "$md5_val" = "$current_md5" ]; then
        group="$group $file_path"
      else
        if [ -n "$group" ]; then
          echo "$group"
        fi
        current_md5="$md5_val"
        group="$file_path"
      fi
    done <<< "$md5_files"
    if [ -n "$group" ]; then
      echo "$group"
    fi
  ) | while IFS= read -r line; do
    echo "$line" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//'
  done | sort
}