csv_to_tsv() {
  local file="$1"
  [[ -z "$file" ]] && return 1

  awk '
  BEGIN {
    RS = ""
    FS = ""
    ORS = "\n"
    output = ""
    field = ""
    in_quotes = 0
    escape_next = 0
    field_count = 0
  }

  function unquote(s,    i, c, len, res, next_c) {
    len = length(s)
    res = ""
    for (i = 1; i <= len; i++) {
      c = substr(s, i, 1)
      if (in_quotes) {
        if (escape_next) {
          if (c == "\"") {
            res = res c
            escape_next = 0
          } else {
            res = res c
            escape_next = 0
          }
        } else {
          if (c == "\"") {
            escape_next = 1
          } else if (c == "\t") {
            res = res "\\t"
          } else if (c == "\n") {
            res = res "\\n"
          } else {
            res = res c
          }
        }
      } else {
        if (c == "\"") {
          in_quotes = 1
          escape_next = 0
        } else {
          if (c == "\t") {
            res = res "\\t"
          } else if (c == "\n") {
            res = res "\\n"
          } else {
            res = res c
          }
        }
      }
    }
    return res
  }

  function parse_field(s,    i, c, len, res, in_q, esc_n) {
    len = length(s)
    res = ""
    in_q = 0
    esc_n = 0
    for (i = 1; i <= len; i++) {
      c = substr(s, i, 1)
      if (in_q) {
        if (esc_n) {
          if (c == "\"") {
            res = res "\""
            esc_n = 0
          } else {
            res = res c
            esc_n = 0
          }
        } else {
          if (c == "\"") {
            esc_n = 1
          } else if (c == "\t") {
            res = res "\\t"
          } else if (c == "\n") {
            res = res "\\n"
          } else {
            res = res c
          }
        }
      } else {
        if (c == "\"") {
          in_q = 1
          esc_n = 0
        } else {
          if (c == "\t") {
            res = res "\\t"
          } else if (c == "\n") {
            res = res "\\n"
          } else {
            res = res c
          }
        }
      }
    }
    return res
  }

  {
    # Read entire file as one block to handle newlines inside quotes
    n = split($0, chars, "")
    field = ""
    in_quotes = 0
    escape_next = 0
    field_num = 1
    output = ""

    for (i = 1; i <= n; i++) {
      c = chars[i]

      # Handle CRLF -> remove CR, keep LF
      if (c == "\r") {
        if ((i < n) && (chars[i+1] == "\n")) {
          # Skip CR, next iteration will see LF
          continue
        } else {
          # Lone CR, skip
          continue
        }
      }

      if (in_quotes) {
        if (escape_next) {
          if (c == "\"") {
            field = field "\""
            escape_next = 0
          } else {
            field = field c
            escape_next = 0
          }
        } else {
          if (c == "\"") {
            escape_next = 1
          } else {
            field = field c
          }
        }
      } else {
        if (c == "\"") {
          in_quotes = 1
          escape_next = 0
        } else if (c == ",") {
          # End of field
          # Append to output
          if (output != "") output = output "\t"
          output = output field
          field = ""
          field_num++
        } else if (c == "\n") {
          # End of record
          if (output != "") output = output "\t"
          output = output field
          print output
          output = ""
          field = ""
          field_num = 1
          in_quotes = 0
          escape_next = 0
        } else {
          field = field c
        }
      }
    }

    # Handle last field if file did not end with newline
    if (field != "" || output != "") {
      if (output != "") output = output "\t"
      output = output field
      print output
      output = ""
    }
  }
  ' "$file"
}