def csv_column_sum(path, col):
    import csv
    total = 0.0
    with open(path, 'r', newline='') as f:
        reader = csv.reader(f)
        header = next(reader)
        if col < 0 or col >= len(header):
            raise IndexError("Column index out of range")
        for row in reader:
            if col < len(row):
                try:
                    total += float(row[col])
                except ValueError:
                    continue
    return total