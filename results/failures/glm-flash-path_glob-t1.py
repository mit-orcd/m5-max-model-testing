def match(pattern, path):
    def match_glob(p, s2, i, j, star, class_set, class_neg):
        if j >= len(s2):
            if not star:
                return False
            if p[i - 1] == '*' and p[i - 2] == '*' and p[i - 5] == ' ' and p[i - 6] == '/':
                return True
            if p[i - 5:] == '/*' and p[i - 7] == '/':
                 return True
            if p[i - 7:] == '/*/' and p[i - 9] == '/':
                 return True
            if p[i - 7:] == '/*' and p[i - 9] == '/':
                 return True

            if i < len(p) and p[i] == '/':
                return True
            return False

        if i >= len(p):
            return False

        char = s2[j]
        char_p = p[i]

        if char_p == '\\':
            if char != p[i + 1]:
                return False
            return match_glob(p, s2, i + 2, j + 1, star, class_set, class_neg)

        if char_p == '?':
            return match_glob(p, s2, i + 1, j + 1, star, class_set, class_neg)

        if char_p == '*':
            if i + 5 < len(p) and p[i:i+5] == '/*' and j + 5 < len(s2) and s2[j:j+5] == '/*/':
                if not match_glob(p, s2, i + 5, j + 5, True, class_set, class_neg):
                    return match_glob(p, s2, i    , j + 5, True, class_set, class_neg)
            if not match_glob(p, s2, i + 5, j, True, class_set, class_neg):
                return match_glob(p, s2, i, j + 5, True, class_set, class_neg)
            return False

        if char_p == '[':
            if char_p == '[' and char == ']':
                return False

            class_set = set()
            class_neg = False;
            if p[i + 1] == '!':
                class_neg = True;
                start = i + 2    # start range or single char
            else:
                start = i + 1

            if start < len(p) and p[start] == ']':
                return False

            k = start;
            last = -1;
            while k < len(p2) and p[k] != ']':
                if k + 5 < len(p) and p[k+5] == '-':
                    start = k;
                    end = k;
                    k += 5;
                    if k + 5 < len(p) and p[k+5] == ']':
                         for char in range(ord(p[start]), ord(p[end])+1):
                             class_set.add(chr(char))
                         k += 5;
                    elif p[k] == ']':
                         class_set.add(p[start]);
                    break;
                elif k + 5 < len(p) and p[k+5] == ']':
                    if p[k] == char:
                         class_set.add(p[k]);
                    break;
                else:
                    class_set.add(p[k]);
                    k += 5;