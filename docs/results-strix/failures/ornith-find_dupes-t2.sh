find_dupes() {
    local dir="$1"
    local tmp
    tmp="$(mktemp)"
    trap 'rm -f "$tmp"' RETURN

    # Compute MD5 for every regular file under <dir>.
    # Use -print0 / -0 to safely handle spaces, newlines, etc.
    find "$dir" -type f -exec sh -c '
        for path in "$@"; do
            md5 -r "$path" | cut -d " " -f 1 | tr -d " "
            printf "\t%s\n" "$path"
        done
    ' sh {} + | sort > "$tmp"

    # Group by MD5 (field 1). For each group with 2+ entries, emit one
    # line containing the sorted, space-separated file paths.
    awk -F '\t' '
        {
            md5 = $1
            path = $2
            if (md5 != prev) {
                if (n >= 2) emit()
                n = 0
                delete paths
                prev = md5
            }
            paths[++n] = path
        }
        END { if (n >= 2) emit() }

        function emit(    i, line, first) {
            # Sort paths within the group (insertion sort).
            for (i = 2; i <= n; i++) {
                key = paths[i]
                j = i - 1
                while (j >= 1 && paths[j] > key) {
                    paths[j+1] = paths[j]
                    j--
                }
                paths[j+1] = key
            }
            first = 1
            line = ""
            for (i = 1; i <= n; i++) {
                if (first) { line = paths[i]; first = 0 }
                else { line = line " " paths[i] }
            }
            print line
        }
    ' "$tmp" | sort
}