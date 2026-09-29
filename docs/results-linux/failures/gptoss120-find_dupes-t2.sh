find_dupes() {
    local dir="${1:?usage: find_dupes <dir>}"
    # Generate hash‑to‑path list
    find "$dir" -type f -print0 |
    while IFS= read -r -d '' file; do
        # md5 -r prints: "<hash> <filename>"
        hash=$(md5 -r "$file" | awk '{print $1}')
        printf '%s\t%s\n' "$hash" "$file"
    done |
    # Sort by hash then by path (paths become sorted within each hash)
    sort -k1,1 -k2,2 |
    # Build groups; output: first_path<TAB>space‑separated‑paths
    awk -F'\t' '
    {
        if ($1 != prev && NR > 1) {
            if (cnt > 1) printf "%s\t%s\n", first, group
            cnt = 0; group = ""; first = ""
        }
        cnt++
        if (cnt == 1) first = $2
        group = (group == "" ? $2 : group " " $2)
        prev = $1
    }
    END {
        if (cnt > 1) printf "%s\t%s\n", first, group
    }' |
    # Sort groups by their first path
    sort -k1,1 |
    # Print only the space‑separated list of paths
    cut -f2-
}