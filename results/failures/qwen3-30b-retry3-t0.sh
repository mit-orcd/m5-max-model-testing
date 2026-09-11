retry3() {
    for i in {1..3}; do
        if "$@"; then
            exit 0
        fi
        last_exit=$?
    done
    exit $last_exit
}