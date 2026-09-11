find_dupes() {
  local dir="$1"
  find "$dir" -type f -exec md5 -r {} \; | sort | uniq -w 32 --group | while read -r _; do
    files=()
    while IFS= read -r line; do
      files+=("${line#* }")
    done
    if (( ${#files[@]} > 1 )); then
      printf "%s\n" "${files[@]}" | sort | tr '\n' ' '
      echo
    fi
  done
}