find_dupes() {
    local dir="$1"
    find "$dir" -type f -exec md5 -r {} + | sort | awk '
        {
            md5 = $1
            file = $2
            if (md5 != prev_md5) {
                if (count >= 2) {
                    for (i = 0; i < count; i++) printf "%s%s", files[i], (i == count-1 ? "\n" : " ")
                }
                delete files
                count = 0
                prev_md5 = md5
            }
            files[count++] = file
        }
        END {
            if (count >= 2) {
                for (i = 0; i < count; i++) printf "%s%s", files[i], (i == count-1 ? "\n" : " ")
            }
        }
    ' | sort
}