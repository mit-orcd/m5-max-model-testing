find_dupes() {
    local dir="$1"

    [ -n "$dir" ] || { echo "usage: find_dupes <dir>" >&2; return 2; }
    [ -d "$dir" ] || { echo "find_dupes: not a directory: $dir" >&2; return 2; }

    local tmp
    tmp="$(mktemp "${TMPDIR:-/tmp}/find_dupes.XXXXXX")" || return 1
    trap 'rm -f "$tmp"' RETURN

    # Compute MD5 for each regular file. BSD md5 prints "<hash>  <path>".
    while IFS= read -r line; do
        # Strip the two-space separator; keep the path intact.
        local hash="${line%%  *}"
        local path="${line#*  }"
        printf '%s\t%s\n' "$hash" "$path"
    done < <(find "$dir" -type f -print0 | xargs -0 -n1 md5 -r) > "$tmp"

    # Group by hash, keep groups with 2+ files, sort paths within group,
    # then sort groups by first path.
    sort -t$'\t' -k1,1 -k2,2 "$tmp" \
    | awk -F'\t' '
        {
            if ($1 == prev) {
                paths[npaths++] = $2
            } else {
                if (npaths >= 2) print_line()
                prev = $1
                npaths = 0
                paths[npaths++] = $2
            }
        }
        END { if (npaths >= 2) print_line() }
        function print_line(   i, out) {
            out = paths[0]
            for (i = 1; i < npaths; i++) out = out " " paths[i]
            print out
        }
    ' \
    | sort
}