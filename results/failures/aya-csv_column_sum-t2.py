import csv


def csv_column_sum(path, col):
    with open(path, 'r') as file:
        reader = csv.reader(file)
        col_index = list(reader).index(list(reader)[0][col])
        sum_value = 0.0
        for row in reader:
            try:
                sum_value += float(row[col_index])
            except ValueError:
                pass
    return sum_value