csv_to_tsv() {
  local file="$1"
  local line
  local field
  local in_quotes=false
  local output_line=""
  local IFS=$'\r\n'

  while IFS= read -r line; do
    local i=0
    local fields=()
    local current_field=""
    local len=${#line}

    while [ $i -lt $len ]; do
      local char="${line:$i:1}"
      if [ "$char" = '"' ]; then
        if [ "$in_quotes" = false ]; then
          in_quotes=true
          ((i++))
          continue
        else
          if [ "${line:$i+1:2}" = '""' ]; then
            current_field="${current_field}$char"
            ((i+=2))
            continue
          fi
          if [ "${line:$i+1:1}" = '"' ]; then
            in_quotes=false
            ((i+=2))
            continue
          fi
        fi
      fi
      if [ "$in_quotes" = false ] && [ "$char" = ',' ]; then
        char='\t'
        in_quotes=false
      fi
      if [ "$in_quotes" = true ] && [ "$char" = $'\n' ]; then
        char='\n'
      fi
      if [ "$char" = $'\n' ]; then
        if [ "$in_quotes" = false ]; then
          fields+=("$current_field")
          current_field=""
          printf '%s\n' "${fields[*]}"
          fields=()
        else
          current_field="${current_field}$char"
        fi
        in_quotes=false
        continue
      fi
      if [ "$char" = $'\r' ]; then
        ((i++))
        continue
      fi
      if [ "$char" = $'\t' ]; then
        char='\t'
      fi
      current_field="${current_field}$char"
      ((i++))
    done

    if [ "$in_quotes" = false ] && [ -n "$current_field" ]; then
      fields+=("$current_field")
      printf '%s\n' "${fields[*]}"
    elif [ "$in_quotes" = true ]; then
      fields+=("$current_field")
      printf '%s\n' "${fields[*]}"
    fi
  done < "$file"
}