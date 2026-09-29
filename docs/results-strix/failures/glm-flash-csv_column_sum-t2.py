import csv

def csv_column_sum(path, col):
    """
    Reads a CSV file, calculates the sum of numeric values in a specified column.
    """
    total = 0.0
    with open(path, 'r') as f:
        reader = csv.reader(f)
        # Skip the header row
        next(reader)
        
        for row in reader:
            if len(row) > col:
                try:
                    value = float(row[col])
                    total += value
                except ValueError:
                    # Skip non-numeric values
                    continue
    return total