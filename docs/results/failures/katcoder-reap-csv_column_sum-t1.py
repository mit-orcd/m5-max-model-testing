def csv_column_sum(path, col):
    total = 0.0
    with open(path, 'r') as f:
        reader = __import__('csv').reader(f)
        header = next(reader)
        for row in reader:
            total += float(row[col])
    return total