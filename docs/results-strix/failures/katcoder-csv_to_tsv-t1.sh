csv_to_tsv() {
  local file="${1:-/dev/stdin}"
  awk '
  BEGIN {
    RS = "\0"
    ORS = ""
    print ""
  }
  {
    gsub(/\r/, "")
    line = $0
    n = length(line)
    i = 0
    out = ""
    while (i <= n) {
      if (i > 0) out = out "\t"
      if (i >= n) {
        # empty field at end
      } else if (substr(line, i, 1) == "\"") {
        # quoted field
        i++
        field = ""
        while (i < n) {
          c = substr(line, i, 1)
          if (c == "\"") {
            if (i + 1 <= n && substr(line, i + 1, 1) == "\"") {
              field = field "\""
              i += 2
            } else {
              i++
              break
            }
          } else if (c == "\n") {
            field = field "\\n"
            i++
          } else if (c == "\t") {
            field = field "\\t"
            i++
          } else {
            field = field c
            i++
          }
        }
        out = out field
      } else {
        # unquoted field
        field = ""
        while (i < n && substr(line, i, 1) != ",") {
          field = field substr(line, i, 1)
          i++
        }
        out = out field
      }
      if (i < n && substr(line, i, 1) == ",") i++
    }
    print out "\n"
  }
  ' "$file"
}