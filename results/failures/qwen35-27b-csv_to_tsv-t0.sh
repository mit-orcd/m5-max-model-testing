csv_to_tsv() {
  local file="$1"
  [[ -z "$file" ]] && return 1

  awk '
  BEGIN {
    FS = ""
    OFS = ""
  }
  {
    # Normalize line endings: remove CR
    gsub(/\r/, "")
    line = $0
    n = length(line)
    field = ""
    in_quotes = 0
    output = ""
    i = 1

    while (i <= n) {
      c = substr(line, i, 1)

      if (in_quotes) {
        if (c == "\"") {
          if (i < n && substr(line, i+1, 1) == "\"") {
            # Escaped quote ""
            field = field "\""
            i++
          } else {
            # End of quoted field
            in_quotes = 0
          }
        } else {
          # Handle special chars inside quotes
          if (c == "\n") {
            field = field "\\n"
          } else if (c == "\t") {
            field = field "\\t"
          } else {
            field = field c
          }
        }
      } else {
        if (c == "\"") {
          in_quotes = 1
        } else if (c == ",") {
          # Field delimiter
          if (output != "") output = output "\t"
          output = output field
          field = ""
        } else {
          field = field c
        }
      }
      i++
    }

    # Append last field
    if (output != "") output = output "\t"
    output = output field

    print output
  }
  ' "$file"
}