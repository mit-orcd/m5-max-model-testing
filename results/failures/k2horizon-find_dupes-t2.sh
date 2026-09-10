find_dupes() {
    local dir="$1"
    [ -d "$dir" ] || return 1

    local md5file tmpfile
    tmpfile=$(mktemp "${TMPDIR:-/tmp}/find_dupes.XXXXXX") || return 1
    trap 'rm -f "$tmpfile"' RETURN

    find "$dir" -type f -print0 2>/dev/null | while IFS= read -r -d '' f; do
        if md5=$(md5 -q "$f" 2>/dev/null); then
            printf '%s\t%s\n' "$md5" "$f" >> "$tmpfile"
        fi
    done

    while IFS=$'\t' read -r md5 path; do
        printf '%s\t%s\n' "$md5" "$path"
    done < "$tmpfile" | awk -F '\t' '
        {
            count[$1]++;
            paths[$1] = paths[$1] " " $2
        }
        END {
            for (h in count) {
                if (count[h] >= 2) {
                    split(paths[h], arr, " ");
                    n = length(arr);
                    for (i = 2; i <= n; i++)
                        for (j = i; j > 1; j--)
                            if (arr[j] < arr[j-1]) {
                                t = arr[j]; arr[j] = arr[j-1]; arr[j-1] = t
                            }
                    first = arr[2];
                    line = arr[2];
                    for (i = 3; i <= n; i++) line = line " " arr[i];
                    printf "%s\t%s\n", first, line
                }
            }
        }
    ' | sort -k1,1 | while IFS=$'\t' read -r first rest; do
        printf '%s\n' "$rest"
    done
}