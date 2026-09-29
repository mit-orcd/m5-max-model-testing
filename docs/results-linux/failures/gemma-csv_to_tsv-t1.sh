csv_to_tsv() {
  awk '
  BEGIN {
    FPAT = "([^,\"]*)|(\"([^\"]|\"\")*\")"
    OFS = "\t"
  }
  {
    for (i = 1; i <= NF; i++) {
      field = $i
      if (field ~ /^\"/) {
        # Remove surrounding quotes
        field = substr(field, 2, length(field) - 2)
        # Unescape double quotes: "" -> "
        gsub(/\"\"/, "\"", field)
        # Escape internal tabs and newlines for TSV compliance
        gsub(/\t/, "\\t", field)
        gsub(/\r/, "", field)
        gsub(/\n/, "\\n", field)
      } else {
        # Remove potential CR from unquoted fields
        gsub(/\r/, "", field)
      }
      printf "%s%s", field, (i == NF ? "" : OFS)
    }
    print ""
  }
  END {
    # Ensure output always ends with one newline if file was not empty
    # (awk print handles this for every record processed)
  }
  ' "$1"
}