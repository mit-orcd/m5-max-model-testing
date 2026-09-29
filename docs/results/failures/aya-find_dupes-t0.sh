function find_dupes() {
    local dir="$1"
    local md5_file="$dir/md5.txt"
    local md5_sum
    local paths
    local path
    local group
    local i=0

    # Create md5 sums file
    find "$dir" -type f -exec md5 -r {} + > "$md5_file"

    # Read md5 sums and group by content
    while IFS= read -r md5_sum; do
        paths=()
        while IFS= read -r path; do
            paths+=("$path")
        done < <(awk -v md5="$md5_sum" '
                $1 == md5 { paths[i++] = $2 }
                END { if (i > 0) print md5, paths[0], paths[i-1] }
            ' "$md5_file")
        if [[ ${#paths[@]} -gt 1 ]]; then
            group=("${paths[@]}")
            echo "${group[@]}"
        fi
    done < "$md5_file"

    # Clean up
    rm "$md5_file"
}