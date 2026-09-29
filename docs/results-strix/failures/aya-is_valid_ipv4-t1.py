def is_valid_ipv4(s):
    if not s.isdigit() and s[0] != '0':
        return True
    if s.count('.') == 3:
        for i in range(4):
            octet = s.split('.')[i]
            if not 0 <= int(octet) <= 255:
                return False
        return True
    return False