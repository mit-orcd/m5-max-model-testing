find_dupes() {
    local dir="$1"
    find "$dir" -type f -print0 | xargs -0 md5 -r | sort | awk '
        {
            md5 = $1
            file = $2
            groups[md5] = (groups[md5] ? groups[md5] " " file : file)
        }
        END {
            for (md5 in groups) {
                split(groups[md5], files, " ")
                if (length(files) >= 2) {
                    n = asort(files)
                    line = ""
                    for (i = 1; i <= n; i++) {
                        line = line (i>1?" ":"") files[i]
                    }
                    print line
                }
            }
        }' | sort
}