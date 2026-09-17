import re


def match(pattern, path):
    pattern = re.sub(r"\\([\[\]{}()^$|*+?])", r"\1", pattern)
    pattern = re.sub(r"\[(-?\d*[-+]\d+)?\]", r"([\w\x{00}-\x{FF}])", pattern)
    pattern = re.sub(r"\.", r"[.\w\x{00}-\x{FF}]", pattern)
    pattern = re.sub(r"\*", r".*", pattern)
    pattern = re.sub(r"\?", r".", pattern)
    pattern = re.sub(r"\{[^}]+\}", r"[^/]*", pattern)
    pattern = re.sub(r"\(", r"(?:", pattern)
    pattern = re.sub(r"\)", r")", pattern)
    pattern = re.sub(r"\[", r"[" + re.escape("[-\w\x{00}-\x{FF}]") + "]", pattern)
    pattern = re.sub(r"\]", r"", pattern)
    pattern = re.sub(r"\^", r"(?!", pattern)
    pattern = re.sub(r"\$", r"(?!\/)$", pattern)
    pattern = re.sub(r"\*", r"(?=/)?", pattern)
    pattern = re.sub(r"\+:", r"(?=/)+:", pattern)
    pattern = re.sub(r"\?", r"(?=/)?", pattern)
    pattern = re.sub(r"\**", r"(?:[^/]*)?", pattern)
    path = re.sub(r"//", "/", path)
    return bool(re.match(pattern, "/" + path))