find_dupes() {
    local dir="$1"
    [ -d "$dir" ] || return 1
    local tmpfile
    tmpfile=$(mktemp)
    # Find all regular files, compute MD5, output "hash<TAB>path"
    find "$dir" -type f -print0 | while IFS= read -r -d '' f; do
        md5 -r "$f" 2>/dev/null | awk -v p="$f" '{print $1 "\t" p}'
    done > "$tmpfile"
    # Group by hash; keep only groups with 2+ files
    awk -F '\t' '
        { hashes[$1]++; paths[$1][++count[$1]] = $2 }
        END {
            for (h in hashes) {
                if (hashes[h] > 1) {
                    # sort paths for this group
                    n = count[h]
                    for (i = 1; i <= n; i++)
                        sorted[i] = paths[h][i]
                    for (i = 1; i <= n; i++)
                        for (j = i+1; j <= n; j++)
                            if (sorted[j] < sorted[i]) {
                                t = sorted[i]; sorted[i] = sorted[j]; sorted[j] = t
                            }
                    line = sorted[1]
                    for (i = 2; i <= n; i++) line = line " " sorted[i]
                    print line > "/dev/stderr"
                    groups[line] = 1
                }
            }
        }
    ' "$tmpfile" 2> "$tmpfile.groups" || return 1
    # Sort groups by first path and print
    sort "$tmpfile.groups"
    rm -f "$tmpfile" "$tmpfile.groups"
}