def csv_column_sum(path, col):
    import csv
    with open(path, 'r') as f:
        reader = csv.reader(f)
        next(reader)  # Skip header
        total = 0.0
        for row in reader:
            try:
                total += float(row[col])
            except (ValueError, IndexError):
                continue
        return total