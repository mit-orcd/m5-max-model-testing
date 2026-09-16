find_dupes() {
    local search_dir="$1"
    find "$search_dir" -type f -exec md5 -r {} + | \
    sort | \
    awk '{
        hash = $1;
        $1 = "";
        sub(/^ /, "", $0);
        print hash " " $0
    }' | \
    awk '{
        hash = $1;
        $1 = "";
        sub(/^ /, "", $0);
        count[hash]++;
        paths[hash] = paths[hash] ? paths[hash] " " $0 : $0;
    }
    END {
        for (h in count) {
            if (count[h] > 1) {
                split(paths[h], arr, " ");
                asort(arr);
                out = "";
                for (i=1; i<=length(arr); i++) {
                    out = out (i==1 ? "" : " ") arr[i];
                }
                print out;
            }
        }
    }' | \
    sort
}