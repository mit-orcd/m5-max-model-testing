def is_valid_ipv4(s):
    if not s:
        return False
    for i, octet in enumerate(s.split('.')):
        if i == 0 and octet == '0':
            continue
        if not octet.isdigit() or not 0 <= int(octet) <= 255:
            return False
    return True