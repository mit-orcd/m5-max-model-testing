def is_valid_ipv4(s):
    s = s.split('.')
    return len(s) == 4 and all(0 <= int(o) <= 255 for o in s) and all(
        not o.startswith('0') or o == '0' for o in s
    )