total_size() {
    local total=0 f size

    if (( $# == 0 )); then
        echo 0
        return
    fi

    while IFS= read -r -d '' f; do
        if [[ -f $f && ! -L $f ]]; then
            size=$(stat -f '%z' -- "$f")
            total=$(( total + size ))
        fi
    done < <(find . -type f -print0)

    echo "$total"
}