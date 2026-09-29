find_dupes() {
    local search_dir="$1"
    find "$search_dir" -type f -exec md5 -r {} + | \
    awk '{
        hash = $1;
        $1 = "";
        sub(/^ /, "", $0);
        paths[$hash] = paths[$hash] ? paths[$hash] " " $0 : $0;
        count[$hash]++;
    }
    END {
        for (h in count) {
            if (count[h] > 1) {
                split(paths[h], arr, " ");
                # Sort paths within group
                asort(arr);
                line = "";
                for (i=1; i<=length(arr); i++) {
                    line = (line == "" ? arr[i] : line " " arr[i]);
                }
                print line;
            }
        }
    }' | sort
}