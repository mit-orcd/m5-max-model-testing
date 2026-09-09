def csv_column_sum(path, col):
    import csv
    total = 0.0
    with open(path, 'r', newline='') as f:
        reader = csv.reader(f)
        next(reader)  # Skip header
        for row in reader:
            total += float(row[col])
    return total