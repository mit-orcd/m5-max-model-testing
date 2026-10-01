import csv

def csv_column_sum(path, col):
    total = 0.0
    with open(path, 'r', newline='') as csvfile:
        reader = csv.reader(csvfile)
        next(reader)  # Skip header row
        for row in reader:
            if col < len(row):
                try:
                    total += float(row[col])
                except ValueError:
                    pass  # Skip non-numeric values
    return total