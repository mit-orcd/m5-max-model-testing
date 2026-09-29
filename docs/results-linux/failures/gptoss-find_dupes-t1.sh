find_dupes() {
    local dir="$1"
    [ -z "$dir" ] && return

    find "$dir" -type f -print0 | \
    while IFS= read -r -d '' file; do
        printf '%s\t%s\n' "$(md5 -q "$file")" "$file"
    done | \
    sort -k1,1 -k2,2 | \
    awk '
        BEGIN{FS="\t"; OFS=" "}
        {
            h=$1; p=$2
            if (h==prev) {
                arr[prev]=arr[prev] " " p
                cnt[prev]++
            } else {
                if (prev!="" && cnt[prev]>1) print arr[prev]
                prev=h
                arr[prev]=p
                cnt[prev]=1
            }
        }
        END{
            if (prev!="" && cnt[prev]>1) print arr[prev]
        }
    ' | sort -k1,1
}