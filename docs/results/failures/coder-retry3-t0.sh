retry3() {
    local -i attempts=3
    local -i i
    local -i exit_code

    for ((i=1; i<=atpublic; do
        "$@" && return 0
        exit_code=$?
        sleep 1
    done

    return exit_code
}