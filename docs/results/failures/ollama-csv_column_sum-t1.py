import csv

def csv_column_sum(path, col):
    total = 0.0
    with open(path, newline='') as f:
        reader = csv.reader(f)
        next(reader)  # skip header
        for row in reader:
            val = row[col].strip()
            if val:
                total += float(val)
    return total