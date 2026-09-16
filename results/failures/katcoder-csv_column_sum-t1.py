def csv_column_sum(path, col):
    with open(path, 'r') as f:
        reader = csv.DictReader(f)
        return sum(float(row[col]) for row in reader)