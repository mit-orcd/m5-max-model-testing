retry3() {
    local cmd=("$@")
    local status
    local attempt=1

    while (( attempt <= 3 )); do
        "${cmd[@]}"
        status=$?
        (( status == 0 )) && exit 0
        (( attempt++ ))
    done

    exit "$status"
}