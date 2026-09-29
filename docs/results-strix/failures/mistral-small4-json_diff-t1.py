import json

def json_diff(a, b, path=''):
    diffs = []
    if isinstance(a, dict) and isinstance(b, dict):
        all_keys = set(a.keys()) | set(b.keys())
        for key in sorted(all_keys):
            new_path = f"{path}.{key}" if path else key
            if key not in a:
                diffs.append(f"{new_path} only in b")
            elif key not in b:
                diffs.append(f"{new_path} only in a")
            else:
                diffs.extend(json_diff(a[key], b[key], new_path))
    elif isinstance(a, list) and isinstance(b, list):
        if a != b:
            diffs.append(path)
    elif a != b:
        diffs.append(path)
    return diffs

def json_diff(a, b):
    if isinstance(a, str):
        a = json.loads(a)
    if isinstance(b, str):
        b = json.loads(b)

    diffs = []
    stack = [(a, b, '')]
    while stack:
        node_a, node_b, current_path = stack.pop()
        if isinstance(node_a, dict) and isinstance(node_b, dict):
            for key in sorted(set(node_a.keys()) | set(node_b.keys)):
                new_path = f"{current_path}.{key}" if current_path else key
                if key not in node_a:
                    diffs.append(f"{new_path} only in b")
                elif key not in node_b:
                    diffs.append(f"{new_path} only in a")
                else:
                    stack.append((node_a[key], node_b[key], new_path))
        elif isinstance(node_a, list) and isinstance(node_b, list):
            if node_a != node_b:
                diffs.append(current_path)
        elif node_a != node_b:
            diffs.append(current_path)
    return sorted(diffs)