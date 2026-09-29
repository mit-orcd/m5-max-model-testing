def csv_column_sum(path, col):
    total = 0.0
    with open(path, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            total += float(row[col])
    return total