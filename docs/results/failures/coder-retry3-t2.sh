```bash
retry3() {
    local -i attempts=3
    local -i i
    local -i exit_code

    for ((i=1; i<=at#>