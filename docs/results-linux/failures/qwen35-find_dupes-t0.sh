find_dupes() {
  local dir="$1"
  if [[ -z "$dir" ]]; then
    echo "Usage: find_dupes <dir>" >&2
    return 1
  fi

  # Find all regular files, compute MD5, format as "md5 path"
  # Use find -type f and md5 -r (BSD syntax)
  find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | \
  awk '{
    # md5 -r output format: "MD5 (file) = hash" or "MD5 file = hash" depending on BSD version
    # Actually, md5 -r on macOS outputs: "MD5 (file) = hash"
    # We need to extract the hash and the filename.
    # Let's parse carefully.
    # Example: "MD5 (./path/to/file) = a1b2c3d4..."
    # Or sometimes just "MD5 file = ..."
    # A robust way: split by " = ", take the hash, then extract filename from parentheses or the rest.
    
    # Let's assume standard BSD output: "MD5 (filename) = hash"
    # If the filename contains spaces, the parentheses handle it.
    
    if (match($0, / = /)) {
      hash = substr($0, RSTART + 3)
      # Remove trailing newline if any
      gsub(/\n/, "", hash)
      
      # Extract filename between parentheses
      if (match($0, /\(.*\)/)) {
        fname = substr($0, RSTART + 1, RLENGTH - 2)
      } else {
        # Fallback if format differs slightly, though unlikely on macOS
        fname = $0
        gsub(/.* = .*/, "", fname)
      }
      
      print hash, fname
    }
  }' | \
  sort -k1,1 -k2,2 | \
  awk '
  {
    hash = $1
    path = $2
    # Reconstruct path if it had spaces (since we split by space in awk)
    # Actually, the previous step printed "hash path", but if path has spaces, this breaks.
    # Better approach: use a delimiter that won't appear in paths, or handle the whole line.
    # Let's restart the logic to be safer with spaces.
  }'
}