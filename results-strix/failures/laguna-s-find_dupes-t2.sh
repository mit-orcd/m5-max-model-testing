find_dupes() {
    local dir="$1"
    [ -d "$dir" ] || return 1
    find "$dir" -type f -exec md5 -r {} + 2>/dev/null | \
    awk '{
        hash=$1; $1=""; sub(/^ /, "");
        files[hash]=files[hash] " " $0
    }
    END {
        for (h in files) {
            n=split(files[h], a, " ")
            if (n >= 3) print files[h]
        }
    }' | \
    while read -r line; do
        echo "$line" | tr ' ' '\n' | grep -v '^$' | sort | tr '\n' ' ' | sed 's/ $//'
    done | sort -k1,1
}