find_dupes() {
  local dir="$1"
  [ -z "$dir" ] && dir="."
  
  # Find all regular files, compute MD5, store in temp file
  # Using md5 -r for BSD compatibility
  find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | \
  awk '{
    # md5 -r output format: "MD5 (file) = hash" or "hash  file" depending on version
    # On macOS, md5 -r typically outputs: "MD5 (file) = hash"
    # We need to robustly parse the hash and the path.
    # Actually, standard macOS md5 output for a file is: "MD5 (filename) = hash"
    # But let's be safe: extract the hash (last field if = is present, or first if not)
    # Let's normalize: we want "hash path"
    
    line = $0
    
    # Check for macOS format: MD5 (file) = hash
    if (match(line, /MD5 \([^)]+\) = ([a-f0-9]+)/, arr)) {
      hash = arr[1]
      # Extract filename between parens
      sub(/.*MD5 \(/, "", line)
      sub(/\) =.*/, "", line)
      path = line
    } else {
      # Fallback or other format: assume "hash  file" or similar
      # Try to find the hash pattern (32 hex chars)
      if (match(line, /([a-f0-9]{32})/, harr)) {
        hash = harr[1]
        # Get the rest as path
        idx = index(line, hash)
        if (idx > 0) {
          path = substr(line, idx + 32)
          gsub(/^[ \t]+/, "", path)
        }
      } else {
        next
      }
    }
    
    if (path != "" && hash != "") {
      print hash, path
    }
  }' | \
  sort -k1,1 -k2,2 | \
  awk '
  {
    hash = $1
    path = $2
    # Handle paths with spaces? The split above is risky if path has spaces.
    # Let's re-parse the line more carefully.
    # Actually, the previous step output "hash path". If path has spaces, awk splits it.
    # We need to reconstruct the path if it had spaces.
    # Better approach: use a unique separator or parse differently.
    
    # Re-doing the parsing logic to be safer for paths with spaces:
    # We will reconstruct the hash and the full path.
  }' 
}