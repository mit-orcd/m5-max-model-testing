csv_col() {
    local col_name="$1"
    python3 - "$col_name" <<'PY'
import sys, csv
col_name = sys.argv[1]
reader = csv.reader(sys.stdin)
header = next(reader, None)
if header is None:
    sys.exit(0)
try:
    idx = header.index(col_name)
except ValueError:
    sys.exit(1)
for row in reader:
    if idx < len(row):
        print(row[idx])
PY
}