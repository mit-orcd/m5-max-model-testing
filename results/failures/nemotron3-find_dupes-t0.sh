find_dupes() {
    local dir="$1"
    find "$dir" -type f -exec md5 -r {} + | sort | awk '
        {
            md5 = $1
            file = substr($0, index($0, $2))
            if (md5 == prev_md5) {
                files[md5] = files[md5] " " file
            } else {
                if (length(prev_md5) > 0 && split(files[prev_md5], arr, " ") >= 2) {
                    n = split(files[prev_md5], arr, " ")
                    for (i = 1; i <= n; i++) {
                        gsub(/^ +| +$/, "", arr[i])
                        if (arr[i] != "") sorted[++c] = arr[i]
                    }
                    for (i = 1; i <= c; i++) {
                        for (j = i + 1; j <= c; j++) {
                            if (sorted[i] > sorted[j]) {
                                temp = sorted[i]
                                sorted[i] = sorted[j]
                                sorted[j] = temp
                            }
                        }
                    }
                    line = ""
                    for (i = 1; i <= c; i++) {
                        line = line (i == 1 ? "" : " ") sorted[i]
                    }
                    print line
                }
                prev_md5 = md5
                files[md5] = file
                delete sorted
                c = 0
            }
        }
        END {
            if (length(prev_md5) > 0 && split(files[prev_md5], arr, " ") >= 2) {
                n = split(files[prev_md5], arr, " ")
                for (i = 1; i <= n; i++) {
                    gsub(/^ +| +$/, "", arr[i])
                    if (arr[i] != "") sorted[++c] = arr[i]
                }
                for (i = 1; i <= c; i++) {
                    for (j = i + 1; j <= c; j++) {
                        if (sorted[i] > sorted[j]) {
                            temp = sorted[i]
                            sorted[i] = sorted[j]
                            sorted[j] = temp
                        }
                    }
                }
                line = ""
                for (i = 1; i <= c; i++) {
                    line = line (i == 1 ? "" : " ") sorted[i]
                }
                print line
            }
        }
    '
}