import csv

def csv_column_sum(path, col):
    with open(path, newline='') as f:
        reader = csv.reader(f)
        next(reader)  # skip header
        total = 0.0
        for row in reader:
            if len(row) > col:
                try:
                    total += float(row[col])
                except ValueError:
                    pass
        return total