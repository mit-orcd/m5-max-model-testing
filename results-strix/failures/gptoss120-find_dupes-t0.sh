find_dupes() {
    if [ -z "$1" ]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    find "$1" -type f -print0 |
    while IFS= read -r -d '' file; do
        hash=$(md5 -r "$file" | awk '{print $1}')
        printf '%s\t%s\n' "$hash" "$file"
    done |
    sort -k1,1 |
    awk -F'\t' '
    function sort_arr(a, n,   i, j, t) {
        for (i = 1; i <= n; i++)
            for (j = i + 1; j <= n; j++)
                if (a[i] > a[j]) { t = a[i]; a[i] = a[j]; a[j] = t }
    }
    {
        if ($1 == prev) {
            cnt++; files[cnt] = $2
        } else {
            if (cnt > 1) {
                sort_arr(files, cnt)
                out = files[1]
                for (i = 2; i <= cnt; i++) out = out " " files[i]
                print out
            }
            prev = $1; cnt = 1; files[1] = $2
        }
    }
    END {
        if (cnt > 1) {
            sort_arr(files, cnt)
            out = files[1]
            for (i = 2; i <= cnt; i++) out = out " " files[i]
            print out
        }
    }' |
    sort -k1,1
}