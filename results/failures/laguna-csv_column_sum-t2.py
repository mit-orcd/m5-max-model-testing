import csv

def csv_column_sum(path, col):
    with open(path, 'r') as f:
        reader = csv.reader(f)
        next(reader)  # Skip header
        total = 0.0
        for row in reader:
            total += float(row[col])
        return total