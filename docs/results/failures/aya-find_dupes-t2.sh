find_dupes() {
    find "$1" -type f -exec md5 -r {} + | sort -k1,1 -o /dev/null | sort -u -k1,1 -o /dev/null | sort -k1,1 | grep -v '^md5' | sed -e 's/^md5 //' -e 's/$/ /' | tr -s ' ' | awk '{for(i=1;i<=NF;i++) print $i}' | sort -o /dev/null | uniq -d -o /dev/null | sort -o /dev/null
}