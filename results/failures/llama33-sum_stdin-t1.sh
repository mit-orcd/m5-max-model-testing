sum_stdin() {
  total=0
  while IFS= read -r line; do
    if [[ $line =~ ^[0-9]+$ ]]; then
      ((total += line))
    fi
  done
  echo "$total"
}