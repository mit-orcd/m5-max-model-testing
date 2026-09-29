def csv_column_sum(path, col):
    import csv

    total = 0.0
    with open(path, newline="") as fh:
        reader = csv.reader(fh)
        next(reader)
        for row in reader:
            total += float(row[col])
    return float(total)