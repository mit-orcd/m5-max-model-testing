def is_valid_ipv4(s):
    s = s.split('.')
    return len(s) == 4 and all(
        0 <= int(o) <= 255 and (o == '0' or not o.startswith('0')) for o in s
    )