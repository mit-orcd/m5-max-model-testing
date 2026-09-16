csv_to_tsv() {
  local file="$1"
  awk -v RS='(?=\r?\n)' '
    function unescape(s) {
      gsub(/\\n/, "\n", s)
      gsub(/\\t/, "\t", s)
      return s
    }
    {
      line = $0
      if (line ~ /\r$/) line = substr(line, 1, length(line) - 1)
      n = split(line, fields, /,\n/)
      out = ""
      i = 1
      while (i <= n) {
        f = fields[i]
        if (f ~ /^"[^"]*"$/ || f ~ /^"[^"]*""[^"]*"$/) {
          # quoted field
          inner = substr(f, 2, length(f) - 2)
          gsub(/""/, "\"", inner)
          inner = unescape(inner)
          out = out (i > 1 ? "\t" : "") inner
        } else {
          out = out (i > 1 ? "\t" : "") f
        }
        i++
      }
      print out
    }
  ' "$file"
}