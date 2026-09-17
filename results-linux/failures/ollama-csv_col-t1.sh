```bash
csv_col() {
    local name="$1"
    local -a header
    IFS=',' read -r -a header