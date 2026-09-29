find_dupes() {
    find "$1" -type f -exec md5 -r {} + | sort | uniq -w32 -D | sort -n | awk '
    {
        if (first_line) {
            print files[0] " " files[1];
            delete files;
            first_line = 0;
        }
        files[NR] = $2;
    }
    END {
        if (NR > 0 && length(files) >= 2) {
            print files[0] " " files[1];
        }
    }';
}