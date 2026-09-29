csv_to_tsv() {
  local file="$1"
  [[ -z "$file" ]] && { echo "Usage: csv_to_tsv <file>" >&2; exit 1; }

  awk '
  BEGIN {
    FS = ""
    OFS = ""
    RS = ""
  }

  function parse_field(str, len,    i, c, nc, quoted, res, ch) {
    res = ""
    quoted = 0
    i = 1
    len = length(str)

    if (len >= 2 && substr(str, 1, 1) == "\"" && substr(str, len, 1) == "\"") {
      quoted = 1
      i = 2
      len = len - 1
    }

    while (i <= len) {
      c = substr(str, i, 1)

      if (quoted) {
        if (c == "\"") {
          i++
          if (i <= len && substr(str, i, 1) == "\"") {
            res = res "\""
            i++
          } else {
            # End of quoted field
            # Consume until next delimiter or end
            # But we handle delimiter outside, so just break
            break
          }
        } else {
          if (c == "\t") {
            res = res "\\t"
          } else if (c == "\n" || c == "\r") {
            res = res "\\n"
          } else {
            res = res c
          }
          i++
        }
      } else {
        if (c == "\"" && i == 1) {
          quoted = 1
          i++
        } else {
          if (c == "\t") {
            res = res "\\t"
          } else if (c == "\n" || c == "\r") {
            res = res "\\n"
          } else {
            res = res c
          }
          i++
        }
      }
    }

    # If we ended in a quoted state without closing quote, treat rest as literal
    # (RFC4180 implies well-formed, but handle edge case)
    # Actually, the loop breaks on closing quote. If not broken, we need to finish.
    # The logic above breaks on closing quote. We need to ensure we consumed it.
    # Re-evaluating logic for simplicity:
    # Let's restart with a more robust state machine approach in main loop.
    return res
  }

  {
    # Normalize line endings: remove CR
    gsub(/\r/, "")
    
    n = split($0, chars, "")
    
    # We need to parse the whole line manually because FS="" splits into chars
    # and we have quotes and escaped newlines logic.
    
    # Reset parsing state
    delete fields
    field_count = 0
    current_field = ""
    in_quotes = 0
    i = 1
    len = length($0)
    
    while (i <= len) {
      c = substr($0, i, 1)
      
      if (in_quotes) {
        if (c == "\"") {
          i++
          if (i <= len && substr($0, i, 1) == "\"") {
            current_field = current_field "\""
            i++
          } else {
            # End of quoted field
            in_quotes = 0
            # Skip the closing quote
            i++
          }
        } else {
          if (c == "\t") {
            current_field = current_field "\\t"
          } else if (c == "\n") {
            current_field = current_field "\\n"
          } else {
            current_field = current_field c
          }
          i++
        }
      } else {
        if (c == "\"" && current_field == "") {
          in_quotes = 1
          i++
        } else if (c == ",") {
          field_count++
          fields[field_count] = current_field
          current_field = ""
          i++
        } else {
          if (c == "\t") {
            current_field = current_field "\\t"
          } else if (c == "\n") {
            current_field = current_field "\\n"
          } else {
            current_field = current_field c
          }
          i++
        }
      }
    }
    
    # Handle last field
    field_count++
    fields[field_count] = current_field
    
    # Output
    for (j = 1; j <= field_count; j++) {
      printf "%s", fields[j]
      if (j < field_count) printf "\t"
    }
    printf "\n"
  }
  ' "$file"
}