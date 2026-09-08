count_matches() {
    local pattern="${1:?missing pattern}"
    local file="${2:?missing file}"
    grep -F -c "$pattern" "$file" 2>/dev/null || echo 0
}