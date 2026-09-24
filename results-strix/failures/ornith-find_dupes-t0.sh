find_dupes() {
    local dir="$1"
    local tmp
    tmp="$(mktemp)"
    trap 'rm -f "$tmp"' RETURN

    # Compute MD5 for every regular file under <dir>.
    # Use -print0 / -0 to be safe with spaces/newlines in names.
    while IFS= read -r -d '' md5; do
        # md5 -r output: "<hash>  <path>"
        printf '%s\n' "$md5"
    done < <(
        find "$dir" -type f -exec md5 -r {} + |
        sed -E 's/^([0-9a-fA-F]{32})  (.*)$/\1 \2/'
    ) > "$tmp"

    # Group by hash, keep groups with 2+ files.
    # For each group, sort the paths, then sort groups by first path.
    awk '{
        hash=$1
        # path is everything after the first two spaces
        path=substr($0, index($0, "  ")+2)
        groups[hash]=groups[hash] (groups[hash]==""?"":"\n") path
        count[hash]++
    }
    END {
        for (h in groups) {
            if (count[h] >= 2) {
                n=split(groups[h], arr, "\n")
                # sort arr
                for (i=1;i<=n;i++)
                    for (j=i+1;j<=n;j++)
                        if (arr[j] < arr[i]) { t=arr[i]; arr[i]=arr[j]; arr[j]=t }
                line=arr[1]
                for (i=2;i<=n;i++) line=line " " arr[i]
                print line
            }
        }
    }' "$tmp" | sort
}