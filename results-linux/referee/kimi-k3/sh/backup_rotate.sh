backup_rotate() {
  ls "$1"/backup-*.tar.gz 2>/dev/null | sort -r | tail -n +"$(( $2 + 1 ))" | sort \
    | while read -r f; do
        rm -f "$f" && basename "$f"
      done
}
