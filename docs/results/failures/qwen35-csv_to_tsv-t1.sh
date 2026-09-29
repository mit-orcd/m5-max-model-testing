csv_to_tsv() {
  local file="$1"
  awk '
  BEGIN { RS = "\n"; FS = "\n" }
  {
    gsub(/\r$/, "")
    n = split($0, buf, "")
    field = ""
    len = 0
    
    for (i = 1; i <= n; i++) {
      c = buf[i]
      if (in_quote) {
        if (c == "\"") {
          if (i < n && buf[i+1] == "\"") {
            field = field "\""
            i++
          } else {
            in_quote = 0
          }
        } else {
          if (c == "\n") {
            # Handle newline inside quoted field by replacing with literal \n
            if (len > 0) field = field "\\n"
            len = 0
          } else {
            field = field c
            len = 0
          }
        }
      } else {
        if (c == "\"") {
          if (len == 0) in_quote = 1
          else field = field c
        } else {
          if (c == ",") {
            if (len == 0 && field == "") {
              # Empty field
            } else {
               # Process field if needed, but here we just append
            }
            # Output current field and tab
            if (len > 0 || field == "") {
               if (output_count > 0) printf "\t"
               printf "%s", field
               output_count++
            }
            field = ""
            len = 0
          } else if (c == "\n") {
            # End of record
            if (output_count > 0) printf "\t"
            printf "%s", field
            printf "\n"
            field = ""
            len = 0
            output_count = 0
            in_quote = 0
          } else {
            field = field c
            len++
          }
        }
      }
    }
    # Handle case where record doesn't end with newline but has content
    if (len > 0 || field != "" || output_count == 0 && field == "") {
       if (output_count > 0) printf "\t"
       printf "%s", field
       printf "\n"
    }
    if (in_quote) {
       # Malformed: quote never closed, treat rest as literal
       if (output_count > 0) printf "\t"
       printf "%s", field
       printf "\n"
    }
  }
  ' "$file"
}