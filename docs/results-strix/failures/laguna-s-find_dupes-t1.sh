find_dupes() {
    local dir="$1"
    local tmp
    tmp=$(mktemp)
    trap 'rm -f "$tmp"' RETURN
    while IFS= read -r -d '' f; do
        printf '%s\0' "$(md5 -r "$f" | cut -d' ' -f1)" "$f"
    done < <(find "$dir" -type f -print0) > "$tmp"
    awk -v RS='\0' -v ORS='\0' '
        {
            if (NR % 2 == 1) {
                hash = $0
            } else {
                files[hash] = files[hash] " " $0
                count[hash]++
            }
        }
        END {
            for (h in count) {
                if (count[h] >= 2) {
                    cmd = "printf \"%s\\0\" " files[h]
                    while ((cmd | getline line) > 0) {
                        gsub(/^ /, "", line)
                        split(line, a, " ")
                        n = asort(a)
                        out = ""
                        for (i = 1; i <= n; i++) out = out " " a[i]
                        sub(/^ /, "", out)
                        print out
                    }
                    close(cmd)
                }
            }
        }
    ' "$tmp" | sort -z | awk -v RS='\0' -v ORS='\n' '
        NF { print }
    ' | sort -k1,1 | awk '
        {
            if (!seen[$1]++) {
                print $0
            }
        }
    '
}