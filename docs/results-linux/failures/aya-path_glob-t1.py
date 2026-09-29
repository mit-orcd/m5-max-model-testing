import re

def match(pattern, path):
    pattern = re.escape(pattern)
    pattern = pattern.replace(r"\?", r".?")
    pattern = pattern.replace(r"\*", r".*?")
    pattern = pattern.replace(r"\{", r"(?:")
    pattern = pattern.replace(r"\}", r")")
    pattern = pattern.replace(r"\[", r"[")
    pattern = pattern.replace(r"\]", r"]")
    pattern = pattern.replace(r"\!", r"[^")
    pattern = "^" + re.sub(r"\(", r"(?:", pattern) + "$"
    return bool(re.match(pattern, path.replace("/", "")))