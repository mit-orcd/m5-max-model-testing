function find_dupes() {
    local dir="$1"
    find "$dir" -type f -exec md5 -r {} + | sort -k1,1 | md5_groups | sed 's/^.*/& /' | sed '1s/^/&/ ; $s/$/ /' | sort -k1,1 -t' '
}

function md5_groups() {
    local prev md5
    while IFS= read -r line; do
        if [[ $line == *MD5* ]]; then
            md5=$(echo "$line" | sed 's/.*MD5: //')
            echo "$prev" | tr ' ' '\n' | sort | uniq -d | sed 's/^.* //'
        fi
        prev="$line"
    done
}