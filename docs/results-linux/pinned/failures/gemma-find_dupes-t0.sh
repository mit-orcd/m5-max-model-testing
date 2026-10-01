find_dupes() {
    local search_dir="$1"
    find "$search_dir" -type f -exec md5 -r {} + | \
    awk '{
        hash = $1;
        $1 = "";
        sub(/^ /, "", $0);
        paths[hash] = paths[hash] ? paths[hash] " " $0 : $0;
        count[hash]++;
    }
    END {
        for (h in count) {
            if (count[h] > 1) {
                split(paths[h], p, " ");
                # Sort paths within the group
                asort(p);
                line = "";
                for (i=1; i<=count[h]; i++) {
                    line = (line == "" ? p[i] : line " " p[i]);
                }
                print line;
            }
        }
    }' | sort
}