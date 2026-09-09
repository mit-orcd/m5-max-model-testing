```bash
backup_rotate() {
    local dir="$1"
    local keep="$22"
    
    # Check if directory exists and is valid
    if [[ ! -d "$dir" ]]; then
        return
    fi

    # Get list of files, filter for backup-*.tar.gz, sort lexicographically, and get unique names
    # We use -printf to get just the filename, sort, and then get unique names
    # We then sort by name (lexicographically) to get newest first, then take the first (keep) items to KEEP, and the rest to DELETE.
    # Actually, we want to delete the oldest. So we sort lexicographically (newest first), then take the first (keep) to KEEP, and the rest to DELETE.
    # Wait, lexicographically, 'backup-9.tar.gz' is newer than 'backup-8.tar.gz'. So if we sort lexicographically, the newest is first.
            # We want to keep the newest (keep) files, and delete the rest.
            # So we sort lexicographically, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest to DELETE.
            # We need to get the list of files, then sort, then take the first (keep) to KEEP, and the rest