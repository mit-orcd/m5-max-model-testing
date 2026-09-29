csv_to_tsv() {
  awk '
  function flush_field() {
    out = field
    gsub(/\\/, "\\\\", out)
    gsub(/\t/, "\\t", out)
    gsub(/\n/, "\\n", out)
    if (nfields > 0) printf "%s", out
    else printf "%s", out
    nfields++
    field = ""
  }
  function flush_record() {
    if (nfields > 0) {
      printf "\n"
      nfields = 0
    }
  }
  BEGIN {
    FS = ""
    in_quotes = 0
    nfields = 0
    have_record = 0
    field = ""
  }
  {
    line = $0
    sub(/\r$/, "", line)
    n = length(line)
    for (i = 1; i <= n; i++) {
      c = substr(line, i, 1)
      if (in_quotes) {
        if (c == "\"") {
          if (i < n && substr(line, i+1, 1) == "\"") {
            field = field "\""
            i++
          } else {
            in_quotes = 0
          }
        } else {
          field = field c
        }
      } else {
        if (c == "\"") {
          in_quotes = 1
        } else if (c == ",") {
          flush_field()
        } else if (c == "\n") {
          flush_field()
          flush_record()
        } else {
          field = field c
        }
      }
    }
    if (in_quotes) {
      field = field "\n"
    }
  }
  END {
    if (in_quotes || nfields > 0 || field != "" ) {
      flush_field()
      flush_record()
    } else if (have_record) {
      flush_record()
    }
  }
  ' "$1"
}