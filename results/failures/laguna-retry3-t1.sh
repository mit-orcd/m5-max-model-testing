retry3() {
    local attempt=1
    local max_attempts=3
    local exit_code=0
    
    while [ $attempt -le $max_attempts ]; do
        "$@" && exit 0
        exit_code=$?
        attempt=$((attempt + 1))
    done
    
    exit $exit_code
}