find_dupes() {
    local dir="$1"
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        md5="$(md5 -q "$file" | awk '{print $NF}')"
        echo "$md5 $file"
    done | sort | awk -F'\t' '{
        if (prev != $1) {
            if (count > 1) {
                for (i = 1; i <= count; i++) {
                    printf "%s ", files[i]
                }
                printf "\n"
            }
            delete files
            count = 0
        }
        count++
        files[count] = $2
        prev = $1
    } END {
        if (count > 1) {
            for (i = 1; i <= count; i++) {
                printf "%s ", files[i]
            }
            printf "\n"
        }
    }' | sort -k1,1
}