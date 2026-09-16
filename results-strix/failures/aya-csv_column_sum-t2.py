import csv

def csv_column_sum(path, col):
    with open(path, 'r') as file:
        reader = csv.reader(file)
        next(reader)  # Skip header
        col_sum = sum(float(row[col]) for row in reader)
    return col_sum