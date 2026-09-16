largest_file() {
  find "$1" -type f -print0 | xargs -0 stat -c '%s %n' | sort -rn | head -n1 | cut -d' ' -f2-
}