<ifm|tool_calls>
<ifm|tool_call>codeOnly
<ifm|arg_key>code</ifm|arg_key>
<ifm|arg_value>import csv

def csv_column_sum(path, col):
    with open(path, newline='') as f:
        reader = csv.reader(f)
        next(reader)  # skip header
        return sum(float(row[col]) for row in reader)
</ifm|arg_value>
</ifm|tool_call>
</ifm|tool_calls>