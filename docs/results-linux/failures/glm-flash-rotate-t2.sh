rotate() {
    if [ -f "$1" ]; then
        for i in $(seq 10 -1 2); do
            if [ -f "$1.$i" ]; then
                mv "$1.$i" "$1.$((i+1))"
            fi
        done
        mv "$1" "$1.1"
    fi
}