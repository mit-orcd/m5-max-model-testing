csv_to_tsv ()
{
  local file="$1"

  awk '
    BEGIN { FS="" }
    function emit() {
      out=""
      for (i=1; i<=nf; i++) {
        if (i>1) out=out "\t"
        out=out f[i]
      }
      print out
    }
    {
      if (in_q) {
        # continuation of a quoted field
        if (substr($0, length($0), 1) == "\\")
          cur = cur $0 "\\CRLF\n"
        else
          cur = cur $0 "\n"
        next
      }
      line = $0
      sub(/\r$/, "", line)
      if ((index(line, "\"") > 0) && (index(line, ",") > 0)) {
        parse(line)
        next
      }
      sub(/,/, "\t", line)
      gsub(/\t/, "\\t", line)
      print line
    }
    function parse(line,   i, c, fc, inq, l) {
      nf=0; cur=""; inq=0
      l=length(line)
      for (i=1; i<=l; i++) {
        c=substr(line, i, 1)
        if (inq) {
          if (c=="\"") {
            fc=substr(line, i+1, 1)
            if (fc=="\"") { cur=cur "\""; i++ }
            else { inq=0 }
          }
          else if (c=="\n") cur=cur "\\n"
          else if (c=="\t") cur=cur "\\t"
          else cur=cur c
        }
        else {
          if (c=="\"") { inq=1; fc="" }
          else if (c==",") { nf++; f[nf]=cur; cur="" }
          else cur=cur c
        }
      }
      nf++; f[nf]=cur
      emit()
    }
  ' "$file"
}