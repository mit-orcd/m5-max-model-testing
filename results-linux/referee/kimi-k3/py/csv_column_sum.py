import csv


def csv_column_sum(path, col):
    total = 0.0
    with open(path, newline="") as f:
        for row in csv.DictReader(f):
            total += float(row[col])
    return total
