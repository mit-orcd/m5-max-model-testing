find_dupes() {
    dir="$1"
    find "$dir" -type f -exec md5 -r {} + | sort -k1,1 -k2,2 | uniq -w32 -d -f1 | awk '{print $2}' | xargs -n1 | sort -u | tr '\n' ' ' | sed 's/ $//' | while read -r line; do
        files=($line)
        if [ ${#files[@]} -ge 2 ]; then
            echo "${files[@]}" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//'
        fi
    done
}