csv_to_tsv() {
  local input_file="$1"
  local buffer=""
  local field=""
  local in_quote= false
  local char=""
  local prev_char=""
  local next_char=""

  while IFS= read -r -n1 char; do
    if [[ -z "$char" ]]; then
      break
    fi

    if [[ "$char" == '"' ]]; then
      if [[ "$in_quote" == true ]]; then
        if [[ "$next_char" == '"' ]]; then
          field+='"'
        else
          in_quote=false
        fi
      else
        in_quote=true
      fi
    elif [[ "$in_quote" == true ]]; then
      if [[ "$char" == $'\t' ]]; then
        field+='\\t'
      elif [[ "$char" == $'\n' ]]; then
        field+='\\n'
      else
        field+="$char"
      fi
      next_char=""
      continue
    else
      if [[ "$char" == $'\t' || "$char" == $'\n' ]]; then
        if [[ -n "$field" ]]; then
          if [[ -z "$buffer" ]]; then
            buffer+="$field"
          else
            buffer+=$'\t'"$field"
            field=""
          fi
        fi
        
        if [[ "$char" == $'\n' || -z "$char" ]]; then
          if [[ -n "$buffer" ]]; then
            echo -n "$buffer"
          fi
          if [[ "$char" != $'\n' ]]; then
            break
          fi
          buffer=""
          field=""
        fi
      else
        field+="$char"
      fi
      next_char=""
    fi
    
    prev_char="$char"
  done < <(while IFS= read -r -n1 char; do printf '%s' "$char"; done < "$input_file")

  if [[ -n "$field" || -n "$buffer" ]]; then
    if [[ -n "$buffer" ]]; then
      buffer+=$'\t'"$field"
    else
      buffer+="$field"
    fi
  fi

  if [[ -n "$buffer" ]]; then
    echo "$buffer"
  fi
}