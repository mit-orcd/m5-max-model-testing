#!/bin/bash
# Wait until no mlx_lm.server is running (queue drained or between phases
# with memory free), then run the seed-oss thinking_budget test once.
while true; do
  if ! pgrep -f "mlx_lm.server" >/dev/null 2>&1; then
    sleep 10
    # double-check the window is real, not a 5s server swap
    if ! pgrep -f "mlx_lm.server" >/dev/null 2>&1 && ! pgrep -f "run-all-missing" >/dev/null 2>&1; then
      /Users/erbmi1/git/m5-max-model-testing/scripts/test-seed-oss-budget.sh
      echo "WATCHER DONE ($(date))"
      exit 0
    fi
  fi
  sleep 300
done
