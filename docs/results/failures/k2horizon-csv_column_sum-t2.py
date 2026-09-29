import csv

def csv_column_sum(path, col):
    with open(path, newline='') as f:
        reader = csv.reader(f)
        next(reader)  # skip header
        return float(sum(float(row[col]) for row in reader))