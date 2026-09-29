csv_to_tsv() {
  local file="$1"
  local line
  local field
  local in_quotes=0
  local output_line=""

  while IFS= read -r line; do
    local output_line=""
    local in_quotes=0
    local field=""

    for ((i=0; i<${#line}; i++)); do
      local char="${line:$i:1}"

      if [[ "$char" == '"' ]]; then
        if [[ $in_quotes -eq 0 ]]; then
          in_quotes=1
        else
          if [[ $i -lt $((${#line}-1)) && "${line:$((i+1)):1}" == '"' ]]; then
            field+='"'
            ((i++))
          else
            in_quotes=0
          fi
        fi
      elif [[ "$char" == ',' && $in_quotes -eq 0 ]]; then
        output_line+=$'\t'
      elif [[ "$char" == $'\r' ]]; then
        continue
      elif [[ "$char" == $'\n' ]]; then
        if [[ $in_quotes -eq 1 ]]; then
          field+=$'\\n'
        else
          output_line+=$'\n'
          echo -n "$output_line"
          output_line=""
        fi
      else
        output_line+="$char"
      fi

      if [[ $in_quotes -eq 1 ]]; then
        field+="$char"
      else
        if [[ $i -eq $((${#line}-1)) ]]; then
          echo -n "$field"
          field=""
        else
          output_line+="$char"
        fi
      fi
    done

    if [[ -n "$field" ]]; then
      echo -n "$field"
    fi

    if [[ -n "$output_line" ]]; then
      echo -n "$output_line"
    fi
  done < "$file"
}