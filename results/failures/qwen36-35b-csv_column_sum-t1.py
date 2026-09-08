import csv

def csv_column_sum(path, col):
    """
    Reads the CSV file at path (first row is a header).
    Returns the sum of the numeric values in column col as a float.
    """
    total = 0.0
    with open(path, 'r', newline='', encoding='utf-8') as f:
        reader = csv.reader(f)
        next(reader)  # Skip header
        for row in reader:
            try:
                total += float(row[col])
            except (ValueError, IndexError):
                continue
    return total