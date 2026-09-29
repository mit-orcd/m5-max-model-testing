find_dupes() {
    dir="$1"
    find "$dir" -type f -exec md5 -r {} + | sort | awk -F'  ' '{print $1, $2}' | uniq -w32 -d | sort -k2 | awk -F'  ' '{print $2}' | xargs -I{} find "$dir" -type f -name "{}" | sort | awk '{a[$0]++} END {for (i in a) if (a[i]>1) print i}' | awk '{print $0}' | sort | awk '{a[$1]++} END {for (i in a) if (a[i]>1) {getline; print $0}}'
}