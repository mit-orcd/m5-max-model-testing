csv_to_tsv() {
    local file="${1:?Missing filename}"
    
    if [[ ! -f "$file" ]]; then
        echo "Error: File '$file' not found." >&2
        return 1
    fi

    python3 -c "
import sys
import csv
import io

def csv_to_tsv(input_file):
    try:
        with open(input_file, 'r', newline='') as f:
            reader = csv.reader(f)
            first = True
            for row in reader:
                if not first:
                    sys.stdout.write('\n')
                first = False
                # Process fields: replace tab and newline with escaped sequences
                processed_fields = []
                for field in row:
                    # Replace literal tab with backslash-t
                    field = field.replace('\t', '\\\\t')
                    # Replace literal newline with backslash-n
                    field = field.replace('\n', '\\\\n')
                    # Note: csv.reader handles quotes, so we don't see quotes in field
                    processed_fields.append(field)
                sys.stdout.write('\t'.join(processed_fields))
            sys.stdout.write('\n')
    except Exception as e:
        print(f'Error: {e}', file=sys.stderr)
        sys.exit(1)

csv_to_tsv(sys.argv[1])
" "$file"
}