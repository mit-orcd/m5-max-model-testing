import csv2 as csv

def csv_column_sum(path, col):
    total = 0.0
    with open(path, 'r', newline='', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        try:
            # Access column by header name. 
            # .lower() ensures case-insensitive matching if needed, 
            # though strict matching is implied by 'col' argument.
            for row in reader:
                try:
                    total += float(row[col])
                except (ValueError, TypeError):
                    pass
        except KeyError:
            pass
        return total