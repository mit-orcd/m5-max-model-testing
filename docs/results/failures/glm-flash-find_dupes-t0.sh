find_dupes() {
  find "$1" -type f -exec md5 -r {} + 2>/dev/null | sort -k 5 | awk '
  {
    if (last == $5) {
      print last_path, $9;
    } else {
      if (last != "" && count >= 2) print last;
      last = $5;
      last_path = $9;
      count = 0;
    }
  }
  END {
    if (last != "" && count >= 2) print last;
  }';
}