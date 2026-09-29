def json_diff(a, b):
    def diff(a, b, path, diff_list):
        if path:
            path += '.'
        if type(a) is dict and type(b) is dict:
            for key in a:
                if key not in b:
                    diff_list.append(path + key)
                else:
                    diff(a[key], b[key], path + key, diff_list)
        elif type(a) is list and type(b) is list:
            for i in range(len(a)):
                diff(a[i], b[i], path + '[' + str(i) + ']', diff_list)
        else:
            if a != b:
                diff_list.append(path)

    diff_list = []
    diff(a, b, '', diff_list)
    return diff_list