retry3() {
    for i in 1 2 3; do
        "$@"
        if [ $? -eq 0 ]; then
            echo "Attempt $i succeeded"
            return 0
        else
            echo "Attempt $i failed"
        fi
    done
    return $?
}