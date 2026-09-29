total_size() {
    local dir=$1
    local sum=0 line n
    while IFS=: read -r n file; do
        # Read -r stops on newline; IFS=: splits first field.
        # We only need the leading number.
        sum=$((sum + n))
    done <<EOF
$(find "$dir" -type f -printf '%s\n' 2>/dev/null)
EOF
    # The above approach is fragile with hostile filenames; use a robust method below.
    :
}