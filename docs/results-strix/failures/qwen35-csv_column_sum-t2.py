def csv_column_sum(path, col):
    total = 0.0
    with open(path, 'r', newline='') as f:
        reader = __import__('csv').reader(f)
        next(reader)  # Skip header
        for row in reader:
            total += float(row[col])
    return total