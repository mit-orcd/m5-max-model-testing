csv_to_tsv() {
  awk '
  function emit() {
    if (nrec > 0) printf "%s\n", line
    line = ""
    nfield = 0
    nrec++
  }
  function push() {
    if (nfield > 0) line = line "\t"
    line = line cur
    nfield++
  }
  BEGIN { FS = "" }
  {
    gsub(/\r$/, "")
    if (infield) {
      if (nl >= 0) {
        if ($0 == "") {
          if (nl < 0) {
            line = line "\n"
            nl = -1
          }
        } else {
          if (nl >= 0) {
            line = line "\n"
            nl = -1
          }
          line = line $0
        }
      }
      next
    }
    i = 1
    len = length($0)
    while (i <= len) {
      c = substr($0, i, 1)
      if (c == ",") {
        push()
        i++
      } else if (c == "\"") {
        i++
        while (i <= len) {
          c = substr($0, i, 1)
          if (c == "\"") {
            if (substr($0, i + 1, 1) == "\"") {
              cur = cur "\""
              i += 2
            } else {
              i++
              break
            }
          } else {
            cur = cur c
            i++
          }
        }
        if (i > len) {
          infield = 1
          nl = 0
          break
        }
        if (substr($0, i, 2) == ",\n" || substr($0, i, 2) == ",\r") {
          i += 2
          push()
        } else {
          while (i <= len) {
            c = substr($0, i, 1)
            if (c == ",") {
              push()
              i++
              break
            } else if (c == "\n" || c == "\r") {
              i++
              break
            } else {
              i++
            }
          }
        }
      } else {
        cur = cur c
        i++
      }
    }
  }
  END {
    if (infield) {
      line = line "\n"
      infield = 0
    }
    push()
    if (nrec > 0) printf "%s\n", line
  }
  ' "$1"
}