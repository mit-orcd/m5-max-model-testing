import csv

def csv_column_sum(path, col):
    total = 0.0
    with open(path, 'r') as f:
        reader = csv.reader(f)
        header = next(reader)
        for row in reader:
            try:
                total += float(row[col])
            except (ValueError, IndexError):
                pass
    return total