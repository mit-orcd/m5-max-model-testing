largest_file() {
  find "$1" -type f -print0 | xargs -0 stat -c '%s %n' | sort -n | tail -n1 | cut -d' ' -f2-
}