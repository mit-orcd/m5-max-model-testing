import csv

def csv_column_sum(path, col):
    with open(path, 'r') as file:
        reader = csv.DictReader(file)
        total = 0.0
        for row in reader:
            value = row.get(col, oke)
            if value and value.replace('.', '').replace('-', '').isdigit():
                total += float(value)
        return total