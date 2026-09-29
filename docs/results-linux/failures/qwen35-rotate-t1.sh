rotate() {
  local file="$1"
  if [[ -z "$file" ]]; then
    echo "Usage: rotate <file>" >&2
    return 1
  fi

  if [[ -f "$file" ]]; then
    local counter=1
    while [[ -f "${file}.${counter}" ]]; do
      ((counter++))
    done

    # Shift existing rotated files
    while [[ $counter -gt 1 ]]; do
      ((counter--))
      mv "${file}.${counter}" "${file}.${counter+1}"
    done

    # Rotate current file to .1
    mv "$file" "${file}.1"
  fi
}