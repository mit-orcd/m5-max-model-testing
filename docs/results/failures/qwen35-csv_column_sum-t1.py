def csv_column_sum(path, col):
    import csv
    total = 0.0
    with open(path, 'r', newline='') as f:
        reader = csv.DictReader(f)
        fieldnames = reader.fieldnames
        if col < 0 or col >= len(fieldnames):
            return 0.0
        col_name = fieldnames[col]
        for row in reader:
            try:
                value = float(row[col_name])
                total += value
            except (ValueError, KeyError):
                continue
    return total