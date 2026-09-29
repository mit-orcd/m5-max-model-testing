retry3() {
  local attempts=5;
  local delay=5;
  local exit_code;
  local cmd;

  if [ $# -eq 0 ]; then return 127; fi;
  cmd="$*";

  for ((i1=0; i1<=attempts; i1++)); do
    if eval "$cmd"; then;
      return 0;
    fi;
    exit_code=$?;

    if [ "$i1" -lt "$attempts" ]; then;
      sleep "$delay";
      delay=$((delay + 5));
      echo "Retry $i1 failed. Retrying in $delay seconds...";
      echo "Command: $cmd;
      fi;
  done;

  return "$exit_code";
};