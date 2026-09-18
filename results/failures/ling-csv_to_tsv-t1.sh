csv_to_tsv() {
  local file="$1"
  local buffer=""
  local in_quote=0
  local c next_c
  local -i crlf=0

  while IFS= read -r -n1 c || [ -n "$c" ]; do
    if [ "$in_quote" -eq 0 ]; then
      if [ "$c" = '"' ]; then
        in_quote=1
        buffer+="$c"
      elif [ "$c" = $'\r' ]; then
        crlf=1
        buffer+=$'\t'
      elif [ "$c" = $'\n' ]; then
        printf '%s\t\n' "$buffer"
        buffer=""
        crlf=0
      elif [ "$c" = ',' ]; then
        buffer+=$'\t'
      else
        buffer+="$c"
      fi
    else
      if [ "$c" = '"' ]; then
        read -r -n1 next_c || next_c=""
        if [ "$next_c" = '"' ]; then
          buffer+='"'
          read -r -n1 c || c=""
        else
          in_quote=0
        fi
      elif [ "$c" = $'\r' ]; then
        crlf=1
        buffer+=$'\\n'
      elif [ "$c" = $'\n' ]; then
        buffer+=$'\\n'
        if [ "$crlf" -eq 1 ]; then
          read -r -n1 c || c=""
          crlf=0
        fi
      elif [ "$c" = $'\t' ]; then
        buffer+=$'\\t'
      else
        buffer+="$c"
      fi
    fi
  done < "$file"

  if [ -n "$buffer" ]; then
    printf '%s\t\n' "$buffer"
  else
    printf '\n'
  fi
}