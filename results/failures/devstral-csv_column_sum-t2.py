def csv_column_sum(path, col):
    with open(path, 'r') as f:
        next(f)  # Skip header
        total = 0.0
        for line in f:
            values = line.strip().split(',')
            if col < len(values):
                try:
                    total += float(values[col])
                except ValueError:
                    pass
    return total