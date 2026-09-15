csv_to_tsv() {
  local file="$1"
  awk '
  BEGIN {
    RS = "\n"
    ORS = "\n"
    recnum = 0
    in_quotes = 0
    field = ""
    rec = ""
  }
  {
    line = $0
    gsub(/\r$/, "", line)
    n = length(line)
    i = 1
    while (i <= n) {
      c = substr(line, i, 1)
      if (in_quotes) {
        if (c == "\"") {
          if (i < n && substr(line, i+1, 1) == "\"") {
            field = field "\""
            i += 2
            continue
          } else {
            in_quotes = 0
            i++
            continue
          }
        } else {
          if (c == "\\") {
            if (i < n) {
              nc = substr(line, i+1, 1)
              if (nc == "n") { c = "\n"; i++ }
              else if (nc == "t") { c = "\t"; i++ }
            }
          }
          field = field c
          i++
        }
      } else {
        if (c == "\"") {
          in_quotes = 1
          i++
        } else if (c == ",") {
          rec = rec field
          if (recnum > 0) rec = rec "\t"
          recnum++
          field = ""
          i++
        } else {
          field = field c
          i++
        }
      }
    }
    rec = rec field
    if (recnum > 0) rec = rec "\t"
    recnum++
    field = ""
  }
  END {
    if (recnum > 0) printf "%s\n", rec
  }
  ' "$file"
}