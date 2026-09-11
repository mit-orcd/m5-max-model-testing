total_size() {
    local dir="$1"
    if [[ ! -d "$dir" ]]; then
        echo 0
        return
    fi
    find "$dir" -type f -print0 2>/dev/null | awk 'BEGIN { total = 0 } { 
        gsub(/\0/, "") 
        cmd = "stat -f %z \"" $0 "\" 2>/dev/null"
        cmd | getline size
        close(cmd)
        if (size != "") total += size
    } END { print total }'
}