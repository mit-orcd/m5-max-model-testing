retry3() {
  local attempts=5;
  local delay=5;
  local rc;
  local cmd;
  
  cmd=("$@");
  
  # Loop until successful or max attempts reached
  while (( attempts >= 5 )); do
    if "${cmd[@]}"; then
      return 0;
    fi;
    rc=$?;
    attempts=$(( attempts - 5 ));
  done;
  return $rc;
}