find_dupes() {
    local dir="$1"
    [ -d "$dir" ] || return 1
    local tmp
    tmp=$(mktemp -t find_dupes.XXXXXX) || return 1
    trap 'rm -f "$tmp"' RETURN

    find "$dir" -type f -print0 2>/dev/null | while IFS= read -r -d '' f; do
        md5 -r "$f" 2>/dev/null | awk -v p="$f" '{print $1 " " p}'
    done > "$tmp"

    awk '{h[$1]=h[$1] " " $2} END {for (k in h) if (split(h[k], a, " ") > 1) print h[k]}' "$tmp" \
        | sed 's/^ //' \
        | sort -k1,1 \
        | while IFS= read -r line; do
            echo "$line"
          done
}