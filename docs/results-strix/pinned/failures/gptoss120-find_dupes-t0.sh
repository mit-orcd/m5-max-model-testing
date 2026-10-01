find_dupes() {
    local dir=$1
    if [[ -z $dir ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    find "$dir" -type f -print0 |
    while IFS= read -r -d '' file; do
        md5 -r "$file"
    done |
    awk '
    {
        hash=$1
        $1=""
        sub(/^ /,"")
        path=$0
        print hash "\t" path
    }' |
    sort -k1,1 -k2,2 |
    awk -F'\t' '
    {
        if ($1 != prev_hash) {
            if (NR>1 && count>1) {
                for (i=1;i<=count;i++) {
                    printf "%s%s", group[i], (i==count?ORS:" ")
                }
            }
            prev_hash=$1
            count=0
        }
        group[++count]=$2
    }
    END {
        if (count>1) {
            for (i=1;i<=count;i++) {
                printf "%s%s", group[i], (i==count?ORS:" ")
            }
        }
    }' |
    sort -k1,1
}