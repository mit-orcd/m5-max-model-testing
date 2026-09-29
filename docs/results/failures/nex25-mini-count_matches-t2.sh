count_matches() {
  grep -Fxc -- "$1" "$2" || true
}