find_dupes() {
    # Ensure a directory argument is given
    [ -z "$1" ] && return 1

    find "$1" -type f -print0 |
    while IFS= read -r -d '' f; do
        # md5 -r prints: "<checksum> <filename>"
        md5 -r "$f"
    done |
    sort |
    awk '
    {
        checksum=$1
        $1=""; sub(/^ +/, "")
        file=$0
        if (checksum == prev) {
            files = files " " file
            cnt++
        } else {
            if (cnt > 1) output_group(prev_files)
            prev = checksum
            prev_files = file
            cnt = 1
        }
    }
    END {
        if (cnt > 1) output_group(prev_files)
    }
    function output_group(list,   n, a, i, out) {
        split(list, a, " ")
        n = asort(a)
        out = a[1]
        for (i = 2; i <= n; i++) out = out " " a[i]
        print out
    }' |
    sort -k1,1
}