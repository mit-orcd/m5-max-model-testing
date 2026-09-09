def is_valid_ipv4(s):
    if s.isdigit() or s == '0':
        return True
    if s.count('.') != 3 or len(s.split('.')) != 4:
        return False
    for octet in s.split('.'):
        if not octet.isdigit() or int(octet) < 0 or int(octet) > 255:
            return False
    return True