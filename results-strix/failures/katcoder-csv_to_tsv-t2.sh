csv_to_tsv() {
  local file="${1:-}"
  [[ -z "$file" ]] && return 1
  [[ ! -f "$file" ]] && return 1

  awk '
  BEGIN {
    RS = "\x00"
    OFS = ""
    first = 1
  }
  {
    # Strip trailing CR
    gsub(/\r$/, "")
    line = $0

    # Remove leading/trailing whitespace around records? No, RFC 4180 does not require it.
    # But we need to parse fields.

    n = length(line)
    i = 1
    field_count = 0
    delete fields
    delete is_quoted

    while (i <= n + 1) {
      if (i > n) {
        # End of line, empty field
        field_count++
        fields[field_count] = ""
        is_quoted[field_count] = 0
        break
      }

      ch = substr(line, i, 1)
      if (ch == "\"") {
        # Quoted field
        field_count++
        is_quoted[field_count] = 1
        field = ""
        i++
        while (i <= n) {
          ch = substr(line, i, 1)
          if (ch == "\"") {
            # Check for escaped quote ""
            if (i + 1 <= n && substr(line, i + 1, 1) == "\"") {
              field = field "\""
              i += 2
            } else {
              # End of quoted field
              i++
              break
            }
          } else {
            field = field ch
            i++
          }
        }
        # Skip comma after quoted field
        if (i <= n && substr(line, i, 1) == ",") {
          i++
        }
        fields[field_count] = field
      } else if (ch == ",") {
        # Empty field
        field_count++
        fields[field_count] = ""
        is_quoted[field_count] = 0
        i++
        # Check if next is end of line
        if (i > n) {
          field_count++
          fields[field_count] = ""
          is_quoted[field_count] = 0
          break
        }
      } else {
        # Unquoted field
        field_count++
        is_quoted[field_count] = 1  # mark as quoted so we escape tabs and newlines
        field = ""
        while (i <= n) {
          ch = substr(line, i, 1)
          if (ch == ",") {
            i++
            break
          }
          field = field ch
          i++
        }
        fields[field_count] = field
      }
    }

    # Output
    if (!first) {
      printf "\n"
    }
    first = 0

    for (j = 1; j <= field_count; j++) {
      if (j > 1) {
        printf "\t"
      }
      val = fields[j]
      if (is_quoted[j]) {
        # Escape backslash first, then tab and newline
        gsub(/\\/, "\\\\", val)
        gsub(/\t/, "\\t", val)
        gsub(/\n/, "\\n", val)
      }
      printf "%s", val
    }
  }
  END {
    if (!first) {
      printf "\n"
    }
  }
  ' "$file"
}