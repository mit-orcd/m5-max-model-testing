find_dupes() {
    dir="$1"
    find "$dir" -type f -exec md5 -r {} + | sort | awk -F'  ' '{print $1, $2}' | \
    awk -F'  ' '{if (seen[$1]++) {print seen[$1], $2} else {seen[$1]=$2}}' | \
    awk '{if (NF>2) {for (i=2; i<=NF; i++) printf "%s ", $i; print ""}}' | \
    sort -k1,1
}