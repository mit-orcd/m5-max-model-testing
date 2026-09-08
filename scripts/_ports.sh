kill_port() {
  local port="$1"
  local pids
  pids=$(lsof -tiTCP:"$port" -sTCP:LISTEN 2>/dev/null || true)
  if [[ -z "${pids}" ]]; then
    return 0
  fi
  echo "Stopping existing server on port ${port} (${pids})..."
  kill ${pids} 2>/dev/null || true
  for _ in $(seq 1 20); do
    if ! lsof -tiTCP:"$port" -sTCP:LISTEN >/dev/null 2>/dev/null; then
      return 0
    fi
    sleep 0.25
  done
  pids=$(lsof -tiTCP:"$port" -sTCP:LISTEN 2>/dev/null || true)
  if [[ -n "${pids}" ]]; then
    kill -9 ${pids} 2>/dev/null || true
    sleep 0.25
  fi
}

wait_http() {
  local url="$1"
  local tries="${2:-60}"
  for _ in $(seq 1 "$tries"); do
    if curl -fsS --max-time 1 "$url" >/dev/null 2>&1; then
      return 0
    fi
    sleep 1
  done
  return 1
}
