retry3() {
    local attempt=0
    local exit_code=0
    while [ $attempt -lt 3 ]; do
        "$@" && exit 0
        exit_code=$?
        attempt=$((attempt + 1))
        if [ $attempt -lt 3 ]; then
            sleep 1
        fi
    done
    echo "Failed after 3 attempts" >&2
    exit $exit_code
}