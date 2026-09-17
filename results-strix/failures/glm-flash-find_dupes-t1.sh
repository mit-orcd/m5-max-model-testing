find_dupes() {
    local dir="$1"
    (
        find "$dir" -type f -exec md5 -r {} + 2>/dev/null | \
        awk '{
            if (md5 && md5 == $1) {
                print file
                print $2
            } else {
                if (md5) print ""
                md5 = $1
                file = $2
            }
        }'
        if [ -n "$md5" ]; then echo ""; fi
    ) | awk '
    NF==0 { next; }
    {
        a[NR]=$0
    }
    END {
        n=asort(a)
        for (i=2; i<=n; i+=2) {
            print a[i] " " a[i+1]
        }
    }'
}