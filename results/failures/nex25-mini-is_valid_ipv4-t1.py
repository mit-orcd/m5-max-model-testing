import re

def is_valid_ipv4(s):
    if not isinstance(s, str):
        return False
    return re.fullmatch(r'(?:0|[1-9][0-9]{0,2})\.('
                        r'0|[1-9][0-9]{0,2})\.('
                        r'0|[1-9][0-9]{0,2})\.('
                        r'0|[1-9][0-9]{0,2})', s) is not None