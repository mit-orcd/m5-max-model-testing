top_freq() {
    local file=$1
    if [[ -z $file || ! -f $file ]]; then
        echo "Usage: top_freq <file>" >&2
        return 1
    fi

    sort "$file" |
        uniq -c |
        sort -nr |
        head -n 10 |
        while IFS= read -r line; do
            count=${line%%[[:space:]]*}
            value=${line#*[[:space:]]}
            printf '%s %s\n' "$count" "$value"
        done
}