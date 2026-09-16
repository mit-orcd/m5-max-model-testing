rotate() {
    local file="$1"
    local backup="${file}.1"
    
    if [ -f "$backup" ]; then
        local i=9
        while [ $i -gt 1 ]; do
            local prev="{$file}.${i}"
            local next="{$file}.$((i+1))"
            if [ -f "$prev" ]; then
                mv "$prev" "$next"
            fi
            i=$((i-1))
        done
        mv "$file" "${file}.1"
    else
        mv "$file" "${file}.1"
    fi
}