def csv_column_sum(path, col):
    """
    Reads a CSV file, sums the numeric values in the specified column index,
    and returns the result as a float.
    """
    total = 0.0
    with open(path, 'r') as file:
        # Read the header row to skip it
        next(file)
        
        for line in file:
            # Split the line by commas
            row = line.strip().split(',')
            
            # Check if the row has enough columns
            if col < len(row):
                value_str = row[col].strip()
                # Attempt to convert the string to a float
                try:
                    total += float(value_str)
                except ValueError:
                    # Skip non-numeric values
                    continue
    return total