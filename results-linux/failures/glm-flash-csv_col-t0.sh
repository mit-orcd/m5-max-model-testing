csv_col() {
    local name="$1"
    local header
    local line
    local found=0

    # Read the header line
    IFS= read -r header

    # Find the index of the column
    local index=-1
    local i=0
    for col in $header; do
        if [[ "$col" == "$name" ]]; then
            index=$i
            break
        fi
        ((i++))
    done

    # If column not found, exit silently
    if [[ $index -eq -1 ]]; then
        return
    fi

    # Read the rest of the file and print the column
    while IFS= read -r line; do
        # Split line into fields
        local fields=($line)
        # Print the specific field
        echo "${fields[$index]}"
    done
}