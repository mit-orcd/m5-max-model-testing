import csv

def csv_column_sum(path, col):
    total = 0.0
    with open(path, 'r') as f:
        reader = csv.reader(f)
        next(reader)  # Skip the header
        for row in reader:
            if len(row) > col:
                try:
                    total += float(row[col])
                except ValueError:
                    pass
    return total