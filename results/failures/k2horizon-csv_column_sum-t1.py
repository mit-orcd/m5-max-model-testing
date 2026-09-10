def csv_column_sum(path, col):
    with open(path, newline='') as f:
        reader = csv.reader(f)
        header = next(reader)
        idx = header.index(col)
        return sum(float(row[idx]) for row in reader)