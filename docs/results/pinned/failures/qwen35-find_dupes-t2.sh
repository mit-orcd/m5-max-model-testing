find_dupes() {
  local dir="$1"
  if [[ -z "$dir" ]]; then
    echo "Usage: find_dupes <dir>" >&2
    return 1
  fi

  # Find all regular files, compute MD5, format as "md5 path"
  # Using find -exec with md5 -r (BSD)
  find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | \
  awk '{
    # md5 -r output format: "MD5 (file) = hash" or "hash  file" depending on version
    # On macOS, md5 -r outputs: "MD5 (file) = hash"
    # We need to extract the hash and the file path.
    # Actually, standard md5 -r on macOS outputs: "MD5 (filename) = hash"
    # Let's parse carefully.
    # Example: MD5 (path/to/file) = a1b2c3d4...
    if (match($0, /MD5 \((.+)\) = ([a-f0-9]+)/, arr)) {
      hash = arr[2]
      path = arr[1]
    } else {
      # Fallback for older or different format if needed, but assume standard BSD
      next
    }
    print hash, path
  }' | \
  sort -k1,1 -k2,2 | \
  awk '{
    if ($1 == prev_hash) {
      paths[NR] = paths[NR-1] " " $2
      count++
    } else {
      if (count > 1) {
        # Sort the paths for this group
        n = split(paths[NR], arr, " ")
        asort(arr)
        line = arr[1]
        for (i = 2; i <= n; i++) {
          line = line " " arr[i]
        }
        print line
      }
      prev_hash = $1
      paths[NR] = $2
      count = 1
    }
  }
  END {
    if (count > 1) {
      n = split(paths[NR], arr, " ")
      asort(arr)
      line = arr[1]
      for (i = 2; i <= n; i++) {
        line = line " " arr[i]
      }
      print line
    }
  }' | \
  sort
}