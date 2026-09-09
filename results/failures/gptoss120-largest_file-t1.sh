largest_file() {
    local dir=$1
    # Find all regular files, print size and path separated by NUL, sort numerically descending, pick the first
    # Use printf with %s and %p to handle spaces and newlines in filenames
    find "$dir" -type f -printf '%s %p\0' |
        sort -z -nr |
        head -z -n1 |
        cut -z -d' ' -f2-
    # The above prints the path followed by a NUL; strip the trailing NUL for clean output
    # (the command substitution below does that)
    # If you prefer a plain newline output without the NUL, you can use:
    # find "$dir" -type f -printf '%s %p\n' | sort -nr | head -n1 | cut -d' ' -f2-
}