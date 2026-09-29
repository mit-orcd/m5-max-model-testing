json_get() {
  local key="$1"
  local value
  local found=false

  while IFS= read -r line; do
    if [[ "$line" =~ ^[[:space:]]*\"$key\"[[:space:]]*:[[:space:]]*\"([^\"]*)\".* ]]; then
      value="${BASH_REMATCH[1]}"
      found=true
    elif [[ "$line" =~ ^[[:space:]]*\"$key\"[[:space:]]*:[[:space:]]*([^,]*) ]]; then
      value="${BASH_REMATCH[1]}"
      found=true
    fi
  done

  if $found; then
    echo "$value"
  else
    echo "null" >&2
    return 1
  fi
}