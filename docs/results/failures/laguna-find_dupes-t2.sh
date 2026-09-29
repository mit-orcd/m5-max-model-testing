find_dupes() {
    local dir="$1"
    local temp_file=$(mktemp)
    trap 'rm -f "$temp_file"' EXIT
    find "$dir" -type f -exec md5 -r {} + | sort -n | awk '{print $1, $2}' > "$temp_file"
    awk '
    {
        if ($1 == prev_hash) {
            files[++count] = $2
        } else {
            if (count >= 2) {
                printf "%s", files[1]
                for (i = 2; i <= count; i++) printf " %s", files[i]
                printf "\n"
            }
            prev_hash = $1
            count = 1
            files[1] = $2
        }
    }
    END {
        if (count >= 2) {
            printf "%s", files[1]
            for (i = 2; i <= count; i++) printf " %s", files[i]
            printf "\n"
        }
    }' "$temp_file"
}