csv_to_tsv() {
  local file="$1"
  awk -v RS="" -v ORS="" '
  {
    gsub(/\r/, "")
    n = split($0, lines, "\n")
    if (lines[n] == "") n--
    
    line = ""
    for (i = 1; i <= n; i++) {
      if (i > 1) line = line "\n"
      line = line lines[i]
    }
    
    if (line == "") next
    
    # Reconstruct the record from split lines
    # We need to handle the multi-line record logic properly
    # Since RS="" handles records separated by blank lines, but we need to handle CRLF/LF correctly
    # Let's switch to char-by-char parsing for robustness with quoted fields containing newlines
    
    delete record
    delete fields
    fcount = 0
    in_quote = 0
    field = ""
    pos = 1
    len = length(line)
    
    for (pos = 1; pos <= len; pos++) {
      c = substr(line, pos, 1)
      
      if (in_quote) {
        if (c == "\"") {
          if (substr(line, pos+1, 1) == "\"") {
            field = field "\""
            pos++
          } else {
            in_quote = 0
          }
        } else {
          field = field c
        }
      } else {
        if (c == "\"") {
          in_quote = 1
        } else if (c == "," || c == "\n") {
          # Escape tabs and newlines in field before appending
          gsub(/\t/, "\\t", field)
          gsub(/\n/, "\\n", field)
          fcount++
          fields[fcount] = field
          field = ""
          if (c == "\n") {
            # End of record
            for (i = 1; i <= fcount; i++) {
              if (i > 1) printf "\t"
              printf "%s", fields[i]
            }
            printf "\n"
            fcount = 0
            delete fields
          }
        } else {
          field = field c
        }
      }
    }
    
    # Handle last field if record didn't end with newline
    if (fcount > 0) {
      gsub(/\t/, "\\t", field)
      gsub(/\n/, "\\n", field)
      fcount++
      fields[fcount] = field
      
      for (i = 1; i <= fcount; i++) {
        if (i > 1) printf "\t"
        printf "%s", fields[i]
      }
      printf "\n"
    }
  }' "$file"
}