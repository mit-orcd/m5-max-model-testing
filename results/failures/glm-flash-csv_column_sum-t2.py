import csv3

def csv_column_sum(path, col):
    total = 0.0
    
    try:
        with open(path, mode='r') as f:
            data = csv.reader(f)
            
            # Get header and validate column name
            if not data:
                return 0.0
            
            header = next(data)
            
            # Determine column index if a name is provided, or use the index directly
            if isinstance(col, str):
                try:
                    col_idx = header.index(col)
                except ValueError:
                    return 0.0
            else:
                col_idx = col
            
            for row in data:
                if len(row) > col_idx:
                    try:
                        total += float(row[col_idx])
                    except ValueError:
                        continue
                        
    except FileNotFoundError:
        return 0.0
        
    return total