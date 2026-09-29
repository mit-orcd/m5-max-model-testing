import csv

def csv_column_sum(path, col):
    """
    Reads the CSV file at path (first row is a header).
    Returns the sum of the numeric values in column col as a float.
    """
    total = 0.0
    
    try:
        with open(path, mode='r', newline='', encoding='utf-8') as file:
            reader = csv.reader(file)
            
            # Skip the header row
            next(reader, None)
            
            for row in reader:
                # Handle empty rows gracefully
                if not row:
                    continue
                
                try:
                    # Check if the column index is valid
                    if col < len(row):
                        val = row[col]
                        # Attempt to convert to float; empty strings result in 0.0
                        total += float(val) if val else 0.0
                except (ValueError, IndexError):
                    continue
                    
    except FileNotFoundError:
        return 0.0
        
    return total