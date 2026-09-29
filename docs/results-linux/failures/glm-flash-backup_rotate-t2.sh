backup_rotate() {
    local dir="$1"
    local keep="$2"
    local count=0
    
    # Count files matching the pattern
    while IFS= read -r -d '' file; do
        ((count++))
    done < <(find "$dir" -maxdepth 1 -type f -name "backup-*.tar.gz" -printf '%f\0' | sort -z -r | sort -z -n)
    
    # If we have fewer than 'keep' files, do nothing
    if (( count <= keep )); then
        return 0
    fi
    
    # Calculate how many to delete
    local to_delete=$(( count - keep ))
    
    # Read filenames in reverse order (newest first) and print/delete the oldest ones
    local current=0
    while IFS= read -r -d '' file; do
        if (( current++ >= to_delete )); then
            # Delete the file
            rm -f "$dir/$file"
            # Print the filename (not path)
            printf '%s\n' "$file"
        fi
    done < <(find "$dir" -maxdepth 1 -type f -name "backup-*.tar.gz" -printf '%f\0' | sort -z -r)
}