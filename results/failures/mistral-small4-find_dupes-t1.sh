find_dupes() {
    find "$1" -type f -exec md5 -r {} + | sort | awk '{
        md5 = $1;
        sub(/^[^ ]+ /, "", $0);
        sub(/  */, "", $0);
        path = $0;
        if (md5 in seen) {
            if (seen[md5] != "") {
                print seen[md5], path;
                seen[md5] = "";
            } else {
                print path;
            }
        } else {
            seen[md5] = path;
        }
    }' | sort -k1,1
}