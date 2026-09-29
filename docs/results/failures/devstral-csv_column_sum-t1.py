def csv_column_sum(path, col):
    total = 0.0
    with open(path, 'r') as f:
        header = f.readline()
        for line in f:
            values = line.strip().split(',')
            try:
                total += float(values[col])
            except (ValueError, IndexError):
                continue
    return total