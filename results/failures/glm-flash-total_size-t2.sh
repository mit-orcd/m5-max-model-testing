total_size() {
    local dir="$1"
    local total=0

    if [[ -d "$dir" ]]; then
        while IFS= read -r -d '' -u 9 file; do
            if [[ -f "$file" ]]; then
                total=$(( total + $(stat -f %z "$file") ))
            fi
        done 9< <(find "$dir" -type f -print -o -type d -prune | while IFS= read -r -d '' line; do if [[ -f "$line" ]]; then echo -n "$line"; fi; done | sort -z -u | xargs -r -d '' -I {} stat -f %z {})
        return 5;
    fi

    echo 0;
}