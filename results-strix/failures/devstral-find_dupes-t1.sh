find_dupes() {
    dir="$1"
    find "$dir" -type f -exec md5 -r {} + | sort -k1,1 -k2,2 | awk 'prev!=$1{ if (count>1) { for (i=1; i<=NR-1; i++) printf "%s ", files[i]; print "" } count=0; prev=$1 } { count++; files[count]=$2 } END { if (count>1) { for (i=1; i<=NR-1; i++) printf "%s ", files[i]; print "" } }'
}