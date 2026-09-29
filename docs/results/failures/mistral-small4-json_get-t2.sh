json_get() {
  local key="$1"
  local line
  while IFS= read -r line; do
    if [[ "$line" =~ ^[[:space:]]*[\"']?$key[\"']?[[:space:]]*:\s*[\"']([^\"']*)[\"'] ]]; then
      echo "${BASH_REMATCH[1]}"
      return 0
    fi
  done
  return 1
}