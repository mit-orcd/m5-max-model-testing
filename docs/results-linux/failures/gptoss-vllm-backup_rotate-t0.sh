```bash
backup_rotate() {
    local dir=$1
    local keep=$2

    # Ensure directory exists and keep is a non‑negative integer
    [[ -d "$dir" ]] || return
    [[ $keep =~ ^[0-9]+$ ]] || return

    # Gather all matching backup files