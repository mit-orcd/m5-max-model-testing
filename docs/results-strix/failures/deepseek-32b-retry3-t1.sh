function retry3() {
    for i in 1 2 3; do
        "$@"
        if [ $? -eq 0 ]; then
            return 0
        fi
    done
    return $?
}