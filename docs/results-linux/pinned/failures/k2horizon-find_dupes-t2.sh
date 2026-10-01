find_dupes() {
    local dir="$1"
    [ -d "$dir" ] || return 1
    local tmp
    tmp=$(mktemp -t find_dupes.XXXXXX) || return 1
    trap 'rm -f "$tmp"' RETURN

    find "$dir" -type f -print0 2>/dev/null | while IFS= read -r -d '' f; do
        md5=$(md5 -q "$f" 2>/dev/null) || continue
        printf '%s\t%s\n' "$md5" "$f"
    done | sort -k1,1 -k2,2 > "$tmp"

    awk -F '\t' '
        { if ($1 != prev) { if (count > 1) print group; group=""; count=0 } prev=$1; group = group ? group " " $2 : $2; count++ }
        END { if (count > 1) print group }
    ' "$tmp"
}