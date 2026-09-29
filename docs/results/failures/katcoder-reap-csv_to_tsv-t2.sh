csv_to_tsv() {
  local file="$1"
  awk '
  BEGIN {
    RS = "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f\x60"
    FS = "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f\x60"
    out = ""
    field_count = 0
    in_quotes = 0
    field = ""
  }
  {
    line = $0
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
        } else if (c == "\\") {
          if (i < n) {
            nc = substr(line, i+1, 1)
            if (nc == "n") {
              field = field "\n"
              i++
            } else if (nc == "t") {
              field = field "\t"
              i++
            } else {
              field = field c
            }
          } else {
            field = field c
          }
        } else if (c == ",") {
          fields[field_count++] = field
          field = ""
        } else if (c == "\n") {
          fields[field_count++] = field
          field = ""
          # end of record
          for (j = 0; j < field_count; j++) {
            f = fields[j]
            gsub(/\\/, "\\\\", f)
            gsub(/\n/, "\\n", f)
            gsub(/\t/, "\\t", f)
            if (j > 0) out = out "\t"
            out = out f
          }
          delete fields
          field_count = 0
          field = ""
          printf "%s\n", out
          out = ""
        } else {
          field = field c
        }
      } else {
        if (c == "\"") {
          in_quotes = 1
        } else if (c == ",") {
          fields[field_count++] = field
          field = ""
        } else if (c == "\n") {
          fields[field_count++] = field
          field = ""
          for (j = 0; j < field_count; j++) {
            f = fields[j]
            gsub(/\\/, "\\\\", f)
            gsub(/\n/, "\\n", f)
            gsub(/\t/, "\\t", f)
            if (j > 0) out = out "\t"
            out = out f
          }
          delete fields
          field_count = 0
          field = ""
          printf "%s\n", out
          out = ""
        } else {
          field = field c
        }
      }
    }
  }
  END {
    if (field_count > 0 || length(field) > 0) {
      fields[field_count++] = field
      for (j = 0; j < field_count; j++) {
        f = fields[j]
        gsub(/\\/, "\\\\", f)
        gsub(/\n/, "\\n", f)
        gsub(/\t/, "\\t", f)
        if (j > 0) out = out "\t"
        out = out f
      }
      printf "%s\n", out
    }
  }
  ' "$file"
}