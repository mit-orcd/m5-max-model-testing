json_get() {
  local key="$1"
  local line
  while IFS= read -r line; do
    if [[ "$line" =~ ^[[:space:]]*\"$key\"[[:space:]]*:[[:space:]]*\"([^\"]*)\" ]]; then
      echo "${BASH_REMATCH[1]}"
      return
    elif [[ "$line" =~ ^[[:space:]]*\"$key\"[[:space:]]*:[[:space:]]*([^[:space:],}]*) ]]; then
      echo "${BASH_REMATCH[1]}"
      return
    fi
  done
}