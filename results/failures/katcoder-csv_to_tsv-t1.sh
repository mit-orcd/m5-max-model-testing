csv_to_tsv() {
  local file="${1:-/dev/stdin}"
  awk '
  BEGIN { RS="\x00"; ORS="" }
  {
    gsub(/\r/, "")
    n = length($0)
    i = 1
    first = 1
    while (i <= n) {
      if (!first) printf "\t"
      if (substr($0, i, 1) == "\"") {
        # quoted field
        i++
        field = ""
        while (i <= n) {
          c = substr($0, i, 1)
          if (c == "\"") {
            if (i+1 <= n && substr($0, i+1, 1) == "\"") {
              field = field "\""
              i += 2
            } else {
              i++
              break
            }
          } else {
            if (c == "\t") c = "\\t"
            else if (c == "\n") c = "\\n"
            field = field c
            i++
          }
        }
        printf "%s", field
      } else {
        # unquoted field
        field = ""
        while (i <= n && substr($0, i, 1) != ",") {
          c = substr($0, i, 1)
          if (c == "\t") c = "\\t"
          else if (c == "\n") c = "\\n"
          field = field c
          i++
        }
        printf "%s", field
      }
      first = 0
      if (i <= n && substr($0, i, 1) == ",") {
        i++
      } else {
        break
      }
    }
    printf "\n"
  }
  ' "$file"
}