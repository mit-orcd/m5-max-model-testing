import json


def parse_json_safely(s):
    try:
        return json.loads(s)
    except (json.JSONDecodeError, TypeError):
        return None
