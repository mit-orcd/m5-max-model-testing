find_dupes() {
    local dir="$1"
    find "$dir" -type f -exec md5 -r {} + | sort -k1,32 | awk '
        $1 != prev {
            if (count >= 2) {
                for (i = 0; i < count; i++) printf "%s%s", files[i], (i == count-1 ? "\n" : " ")
            }
            prev = $1
            count = 0
            delete files
        }
        {
            files[count++] = $2
        }
        END {
            if (count >= 2) {
                for (i = 0; i < count; i++) printf "%s%s", files[i], (i == count-1 ? "\n" : " ")
            }
        }
    '
}