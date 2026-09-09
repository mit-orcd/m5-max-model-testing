find_dupes() {
    local root="$1"

    [ -n "$root" ] || { echo "usage: find_dupes <dir>" >&2; return 2; }
    [ -d "$root" ] || { echo "find_dupes: not a directory: $root" >&2; return 2; }

    (
        cd "$root" || exit 1

        # MD5 + NUL-terminated path for every regular file (recursively).
        while IFS= read -r -d '' f; do
            md5 -r "$f"
        done < <(find . -type f -print0) \
        | sed 's/^[0-9a-f]\{32\}  \.\//  /' \
        | LC_ALL=C sort -k1,1 -k2 \
        | awk '{
            key = $1
            if (key == prev) {
                paths[key] = paths[key] " " $2
                count[key]++
            } else {
                paths[key] = $2
                count[key] = 1
                prev = key
            }
        }
        END {
            n = 0
            for (k in paths)
                if (count[k] >= 2)
                    lines[n++] = paths[k]
            for (i = 0; i < n; i++)
                print lines[i]
        }' \
        | LC_ALL=C sort
    )
}