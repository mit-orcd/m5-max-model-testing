find_dupes() {
    local dir="$1"
    find "$dir" -type f -exec md5 -r {} \; | sort | awk '
    BEGIN { prev_md5 = ""; group = "" }
    {
        md5 = $1
        file = $2
        if (md5 == prev_md5) {
            group = group " " file
        } else {
            if (length(group) > 0) {
                n = split(substr(group, 2), files, " ")
                if (n >= 2) {
                    asort(files)
                    for (i = 1; i <= n; i++) printf "%s%s", files[i], (i < n ? " " : "\n")
                }
            }
            group = file
            prev_md5 = md5
        }
    }
    END {
        if (length(group) > 0) {
            n = split(substr(group, 2), files, " ")
            if (n >= 2) {
                asort(files)
                for (i = 1; i <= n; i++) printf "%s%s", files[i], (i < n ? " " : "\n")
            }
        }
    }
    '
}