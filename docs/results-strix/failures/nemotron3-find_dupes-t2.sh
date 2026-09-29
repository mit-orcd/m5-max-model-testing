find_dupes() {
    local dir="$1"
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        md5 -r "$file" | awk '{print $1}'
    done | sort | uniq -d | while read -r hash; do
        find "$dir" -type f -exec bash -c 'md5 -r "$1" | cut -d" " -f1' _ {} \; |
            grep -Fx "$hash" | while read -r f; do
                printf '%s\0' "$f"
            done | sort -z | tr '\0' ' ' | sed 's/ $/\n/'
    done | sort
}