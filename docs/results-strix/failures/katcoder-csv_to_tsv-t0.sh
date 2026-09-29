csv_to_tsv() {
  local file="${1:-/dev/stdin}"
  awk '
  BEGIN {
    RS = "\0"
    ORS = ""
  }
  {
    gsub(/\r/, "")
    s = $0
    n = length(s)
    i = 0
    line = ""
    while (i <= n) {
      if (i > 0) line = line "\t"
      if (i >= n) {
        # trailing empty field or end
      } else if (substr(s, i, 1) == "\"") {
        # quoted field
        i++
        field = ""
        while (i < n) {
          c = substr(s, i, 1)
          if (c == "\"") {
            if (i + 1 <= n && substr(s, i + 1, 1) == "\"") {
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
        line = line field
      } else {
        # unquoted field
        field = ""
        while (i < n && substr(s, i, 1) != "," && substr(s, i, 1) != "\n") {
          field = field substr(s, i, 1)
          i++
        }
        line = line field
      }
      # skip comma
      if (i < n && substr(s, i, 1) == ",") {
        i++
      }
      # if we hit newline, end of record
      if (i < n && substr(s, i, 1) == "\n") {
        print line "\n"
        line = ""
        i++
      }
    }
    # if there is remaining line content (no trailing newline on last record)
    if (line != "" || i > 0) {
      # check if we processed any content
      if (length(s) > 0) {
        print line "\n"
      }
    }
  }
  ' "$file"
}