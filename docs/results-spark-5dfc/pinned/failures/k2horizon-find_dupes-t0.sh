find_dupes() {
    local dir="$1"
    [ -d "$dir" ] || return 1
    local tmp
    tmp=$(mktemp -t find_dupes.XXXXXX) || return 1
    trap 'rm -f "$tmp"' RETURN

    find "$dir" -type f -print0 2>/dev/null | while IFS= read -r -d '' f; do
        md5 -r "$f" 2>/dev/null | awk -v path="$f" '{print $1 " " path}'
    done > "$tmp"

    awk '{ if (NF >= 2) { h[$1] = h[$1] " " $2 } } END { for (k in h) print k h[k] }' "$tmp" \
        | sed 's/^[0-9a-f]* //' \
        | awk '{ n=split($0,a," "); for(i=1;i<=n;i++) print a[i] }' \
        | sort \
        | awk '{ groups[NR] = $0; if (NR > 1 && $0 == prev) { groups[NR-1] = groups[NR-1] " " $0 } else { if (NR > 1) print groups[NR-1]; groups[NR] = $0 } prev = $0 } END { if (NR >= 1) print groups[NR] }' \
        | awk 'NF > 1' \
        | sort -k1,1
}