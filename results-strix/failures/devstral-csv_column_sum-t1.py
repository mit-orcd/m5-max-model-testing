def csv_column_sum(path, col):
    with open(path, 'r') as file:
        next(file)  # Skip header
        total = 0.0
        for line in file:
            row = line.strip().split(',')
            try:
                total += float(row[col])
            except (ValueError, IndexError):
                continue
    return total