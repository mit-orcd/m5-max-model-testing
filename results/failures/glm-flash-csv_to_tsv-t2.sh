csv_to_tsv() {
  local input_file="$1"
  local line_buffer=""
  local in_quote=0
  local c

  while IFS= read -r -n1 c; do
    case "$c" in
      '"')
        if [ "$in_quote" = "0" ]; then
          in_quote=1
        else
          line_buffer+="$c"
        fi
        ;;
      ',')
        if [ "$in_quote" = "0" ]; then
          printf '\t'
        else
          line_buffer+="$c"
        fi
        ;;
      $'\t')
        if [ "$in_quote" = "0" ]; then
          break  # End of field, expecting end of line or separator or unquoted quote next
        else
          line_buffer+="$c"
        fi
        ;;
      $'\n')
        if [ "$in_quote" = "0" ]; then
          if [ -n "$line_buffer" ]; then
             printf '%s' "$line_buffer"
             line_buffer=""
          fi
          break  # End of line, processing complete for this record
        else
          line_buffer+="$c"
        fi
        ;;
      '')
        # EOF or broken pipe
        if [ "$in_quote" = "0" ]; then
            break;
        else
            # If we are in a quote and reach EOF, treat it as a newline to close the field (or quote)
            break;
        fi
        ;;
      *)
        line_buffer+="$c"
        ;;
    esac

  done < "$input_file"

  if [ -n "$line_buffer" ] || [ -n "${BASH_REMATCH:-}" ]; then
    # Handle potential last line without newline by simulating flush if not empty, or just outputting buffer.
    # The loop effectively processes until EOF. If the file ended with a newline, the last record is already processed.
    # If it ended without newline, the buffer holds the last record.
    # However, the loop breaks on newline or tab (if quoted). 
    # We need to output the buffer if it's not empty.
    
    # A simplified flush logic based on the loop structure:
    # The loop accumulates. It breaks on newline (if not quoted) or tab (if quoted, which shouldn't happen for a valid record unless we are in the middle of a line, which is impossible).
  fi

  # The loop logic has a flaw: it breaks on tab if quoted, which is never reached.
  # We need to process the buffer and print it if it's not empty.
  # Actually, the loop breaks on newline (if not quoted) or EOF.
  # We need to print the buffer if it has content.
  if [ -n "$line_buffer" ]; then
     printf '%s' "$line_buffer"
     printf '\n'
  fi
}

# Rewrite for clarity and correctness

csv_to_tsv() {
  local line=""
  local c=""
  local quote=0
  local buffer=""
  
  while IFS= read -r -n 1 c; do
    # Handle EOF silently if it hits immediately, though loop handles it.
    
    case "$c" in
      '"')
        if [ "$quote" -eq  9 ]; then # 9 is tab, split char case in loop? No, read is char by char.
            # This approach is too complex. Use a state machine.
            pass;
            ;;
  esac;
}