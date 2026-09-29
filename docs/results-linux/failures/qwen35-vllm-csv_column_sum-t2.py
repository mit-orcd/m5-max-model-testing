def csv_column_sum(path, col):
    total = 0.0
    with open(path, 'r') as f:
        lines = f.readlines()
        if not lines:
            return total
        header = lines[0].strip().split(',')
        if col < 0 or col >= len(header):
            raise IndexError("Column index out of range")
        
        col_index = header.index(header[col]) if isinstance(col, str) else col
        
        for line in lines[1:]:
            if not line.strip():
                continue
            values = line.strip().split(',')
            if col_index < len(values):
                try:
                    total += float(values[col_index])
                except ValueError:
                    continue
    return total