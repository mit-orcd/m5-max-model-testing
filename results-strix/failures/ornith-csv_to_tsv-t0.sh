csv_to_tsv() {
  local file="$1"
  awk '
  BEGIN { RS="\n"; FS="" }
  {
    sub(/\r$/, "")
    if (nrec > 0) printf "\n"
    nrec++
    line = ""
    nf = 0
    i = 1
    L = length($0)
    while (i <= L + 1) {
      c = substr($0, i, 1)
      if (c == "" || c == "\n") {
        break
      }
      if (nf > 0) {
        line = line "\t"
      }
      if (c == "\"") {
        field = ""
        i++
        while (i <= L) {
          c = substr($0, i, 1)
          if (c == "\"") {
            if (substr($0, i+1, 1) == "\"") {
              field = field "\""
              i += 2
            } else {
              i++
              break
            }
          } else {
            field = field c
            i++
          }
        }
        if (substr($0, i, 1) == ",") {
          i++
        }
      } else {
        field = ""
        while (i <= L) {
          c = substr($0, i, 1)
          if (c == ",") {
            break
          }
          field = field c
          i++
        }
        if (substr($0, i, 1) == ",") {
          i++
        }
      }
      gsub(/\\/, "\\\\", field)
      gsub(/\t/, "\\t", field)
      gsub(/\n/, "\\n", field)
      line = line field
      nf++
    }
    printf "%s", line
  }
  END { printf "\n" }
  ' "$file"
}