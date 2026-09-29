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
        NR > 1 && $1 != prev {
            if (count > 1) {
                for (i = 1; i <= count; i++) printf "%s%s", paths[i], (i < count ? " " : "\n")
            }
            count = 1
            paths[1] = $2
            prev = $1
        }
        NR == 1 { count = 1; paths[1] = $2; prev = $1 }
        NR > 1 && $1 == prev { count++; paths[count] = $2 }
        END {
            if (count > 1) {
                for (i = 1; i <= count; i++) printf "%s%s", paths[i], (i < count ? " " : "\n")
            }
        }
    ' "$tmp"
}