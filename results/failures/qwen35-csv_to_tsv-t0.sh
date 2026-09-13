csv_to_tsv() {
  local file="$1"
  local in_quote=0
  local field=""
  local record=""
  local i=0
  local len
  local char
  local next_char

  if [[ ! -f "$file" ]]; then
    return 1
  fi

  # Read file character by character to handle embedded newlines and quotes correctly
  while IFS= read -r -n1 char || [[ -n "$char" ]]; do
    if [[ $in_quote -eq 1 ]]; then
      if [[ "$char" == '"' ]]; then
        # Check for escaped quote ""
        if [[ -n "$next_char" && "$next_char" == '"' ]]; then
          field+='"'
          # Consume next char
          read -r -n1 next_char 2>/dev/null || next_char=""
          continue
        else
          # End of quoted field
          in_quote=0
          continue
        fi
      else
        field+="$char"
        continue
      fi
    fi

    # Not in quote
    if [[ "$char" == '"' ]]; then
      in_quote=1
      continue
    elif [[ "$char" == $'\n' ]]; then
      # Handle CRLF: if previous char was CR, ignore it (already handled by read -r logic usually, but explicit check)
      # Actually read -r -n1 handles CR/LF as separate chars if not using -d.
      # We need to handle CRLF input where CR might be left over.
      # Standard read -r -n1 on Linux with CRLF input: char='\r', next='\n'.
      # If char is CR, skip it.
      if [[ "$char" == $'\r' ]]; then
        continue
      fi
      
      # End of record
      # Process current field
      if [[ -n "$field" || $i -eq 0 ]]; then
         # If field is empty string but we are at start of record, it's an empty field
         # If we have accumulated chars, add them.
         # Handle escaping for output: \n -> \n, \t -> \t
         local escaped_field=""
         local f_len=${#field}
         local f_i=0
         while [[ $f_i -lt $f_len ]]; do
           local f_char="${field:$f_i:1}"
           if [[ "$f_char" == $'\n' ]]; then
             escaped_field+='\n'
           elif [[ "$f_char" == $'\t' ]]; then
             escaped_field+='\t'
           else
             escaped_field+="$f_char"
           fi
           ((f_i++))
         done
         
         if [[ -n "$record" ]]; then
           record+=$'\t'
         fi
         record+="$escaped_field"
      fi
      
      # Output record
      echo "$record"
      
      # Reset
      record=""
      field=""
      i=0
      continue
    elif [[ "$char" == $'\r' ]]; then
      # Skip CR, it's part of CRLF or just CR
      continue
    elif [[ "$char" == ',' ]]; then
      # End of field
      if [[ -n "$record" ]]; then
        record+=$'\t'
      fi
      record+="$field"
      field=""
      ((i++))
      continue
    else
      field+="$char"
    fi
  done < "$file"

  # Handle last field/record if file doesn't end with newline
  if [[ -n "$field" || $i -gt 0 || -n "$record" ]]; then
    if [[ -n "$record" ]]; then
      record+=$'\t'
    fi
    record+="$field"
    echo "$record"
  elif [[ -n "$record" ]]; then
    echo "$record"
  fi
}