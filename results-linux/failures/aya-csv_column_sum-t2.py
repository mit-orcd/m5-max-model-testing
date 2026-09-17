import csv

def csv_column_sum(path, col):
    total = 0.0
    with open(path, 'r') as file:
        reader = csv.reader(file)
        next(reader)  # Skip header
        for row in reader:
            total += float(row[col]) if row[col] else 0.0
    return total