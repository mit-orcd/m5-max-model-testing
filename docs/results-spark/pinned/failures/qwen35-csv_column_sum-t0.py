import csv

def csv_column_sum(path, col):
    total = 0.0
    with open(path, 'r', newline='') as f:
        reader = csv.reader(f)
        next(reader)  # Skip header
        for row in reader:
            if col < len(row):
                try:
                    total += float(row[col])
                except ValueError:
                    continue
    return total