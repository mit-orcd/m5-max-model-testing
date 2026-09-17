def json_diff(a, b):
    def _diff(current_a, current_b, path):
        changes = []
        
        # Check if both are dicts
        if isinstance(current_a, dict) and isinstance(current_b, dict):
            # Keys present in both
            all_keys = set(current_a.keys()) | set(current_b.keys())
            for key in sorted(all_keys):
                new_path = f"{path}.{key}" if path else key
                if key in current_a and key in current_b:
                    changes.extend(_diff(current_a[key], current_b[key], new_path))
                elif key in current_a:
                    changes.append(new_path)
                else:
                    changes.append(new_path)
                    
        # Check if both are lists
        elif isinstance(current_a, list) and isinstance(current_b, list):
            length = max(len(current_a), len(current_b))
            for i in range(length):
                new_path = f"{path}[{i}]"
                if i < len(current_a) and i < len(current_b):
                    changes.extend(_diff(current_a[i], current_b[i], new_path))
                elif i < len(current_a):
                    changes.append(new_path)
                else:
                    changes.append(new_path)
                    
        # Scalars (or mismatched types)
        else:
            if current_a != current_b:
                changes.append(path)
                
        return changes

    return sorted(_diff(a, b, ""))