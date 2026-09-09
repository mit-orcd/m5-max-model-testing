def is_valid_ipv4(s):
    if s.count('.') != 3 or len(s) != 12:
        return False
    for i in range(4):
        octet = s[i * 3:i * 3 + 3]
        if not octet[0] == '0' and int(octet) > 255:
            return False
        if octet[0] == '0' and octet != '0':
            return False
    return True