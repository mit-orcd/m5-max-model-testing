def csv_column_sum(path, col):
    with open(path, 'r') as f:
        reader = csv.reader(f)
        header = next(reader)
        col_index = header.index(col)
        total = 0.0
        for row in reader:
            total += float(row[col_index])
    return total