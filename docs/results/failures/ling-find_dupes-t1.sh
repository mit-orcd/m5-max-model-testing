find_dupes() {
    (
        cd "$1" || return 1
        md5_files=$(find . -type f -exec md5 -r {} + | sort -k1,1)
        echo "$md5_files" | awk '
        {
            md5 = $1
            $1 = ""
            sub(/^ /, "", $0)
            files[md5] = files[md5] (files[md5] ? " " : "") $0
            count[md5]++
        }
        END {
            n = 0
            for (md5 in files) {
                if (count[md5] >= 2) {
                    n++
                    split(files[md5], arr, " ")
                    asort(arr)
                    sorted = arr[1]
                    for (i = 2; i <= length(arr); i++) {
                        sorted = sorted " " arr[i]
                    }
                    groups[n] = sorted
                    first[n] = arr[1]
                }
            }
            for (i = 1; i <= n; i++) {
                for (j = i + 1; j <= n; j++) {
                    if (first[i] > first[j]) {
                        tmp = groups[i]; groups[i] = groups[j]; groups[j] = tmp
                        tmp = first[i]; first[i] = first[j]; first[j] = tmp
                    }
                }
            }
            for (i = 1; i <= n; i++) {
                print groups[i]
            }
        }'
    )
}