csv_to_tsv() {
  local infile=$1
  awk '
    function flush() {
      if (recn > 0) {
        for (i = 1; i <= recn; i++) {
          if (i > 1) printf "\t"
          printf "%s", fields[i]
        }
        printf "\n"
        recn = 0
        delete fields
      }
    }
    {
      sub(/\r$/, "")
      line = $0
      n = length(line)
      i = 1
      recn++
      fin = 1
      while (1) {
        if (i > n) {
          print "error: unterminated quoted field" > "/dev/stderr"
          exit 1
        }
        c = substr(line, i, 1)
        if (c == "\"") {
          i++
          while (1) {
            if (i > n) {
              print "error: unterminated quoted field" > "/dev/stderr"
              exit 1
            }
            c = substr(line, i, 1)
            if (c == "\"") {
              if (i < n && substr(line, i+1, 1) == "\"") {
                fin = fin "\""
                i += 2
              } else {
                i += 2
                break
              }
            } else {
              fin = fin c
              i++
            }
          }
        } else if (c == ",") {
          fields[recn] = fin
          fin = ""
          recn++
          i++
        } else {
          if (c == "\t") fin = fin "\\t"
          else if (c == "\n") fin = fin "\\n"
          else fin = fin c
          i++
        }
      }
      if (fin != "") {
        fields[recn] = fin
        recn++
      }
    }
    END { flush() }
  ' "$infile"
}