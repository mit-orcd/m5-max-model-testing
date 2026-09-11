```bash
sum_stdin() {
  local sum=0
  while IFS= read -r line; do
    stripped