find_dupes() {
    dir="$1"
    find "$dir" -type f -exec md5 -r {} + | \
    awk '{
        if ($0 != prev) {
            if (count > 1) {
                for (i=1; i<=count; i++) {
                    printf "%s ", files[i];
                }
                print "";
            }
            delete files;
            count = 0;
        }
        count++;
        files[count] = $2;
        prev = $0;
    }
    END {
        if (count > 1) {
            for (i=1; i<=count; i++) {
                printf "%s ", files[i];
            }
            print "";
        }
    }' | \
    sort -k1,1
}