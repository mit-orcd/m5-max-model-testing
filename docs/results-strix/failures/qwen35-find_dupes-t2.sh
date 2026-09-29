find_dupes() {
  local dir="${1:-.}"
  
  # Ensure directory exists
  if [[ ! -d "$dir" ]]; then
    echo "Error: '$dir' is not a directory" >&2
    return 1
  fi

  # Find all regular files, compute md5 (BSD format), sort by hash then path
  # Using find -type f and a while loop to handle paths safely
  while IFS= read -r -d '' file; do
    # BSD md5 -r outputs: "MD5 (file) = hash"
    local hash
    hash=$(md5 -r "$file" | awk '{print $NF}')
    printf '%s\t%s\n' "$hash" "$file"
  done < <(find "$dir" -type f -print0) | \
  sort -t$'\t' -k1,1 -k2,2 | \
  awk -F'\t' '
    {
      if ($1 == prev_hash) {
        # Append current file to the current group
        groups[prev_hash] = groups[prev_hash] " " $2
        count[prev_hash]++
      } else {
        # New hash group
        prev_hash = $1
        groups[prev_hash] = $2
        count[prev_hash] = 1
      }
    }
    END {
      # Iterate through hashes to find groups with duplicates
      for (hash in count) {
        if (count[hash] > 1) {
          # Split the group string into an array to sort paths
          n = split(groups[hash], paths, " ")
          # Sort the paths (bubble sort for simplicity in awk)
          for (i = 1; i <= n; i++) {
            for (j = i + 1; j <= n; j++) {
              if (paths[i] > paths[j]) {
                tmp = paths[i]
                paths[i] = paths[j]
                paths[j] = tmp
              }
            }
          }
          # Build the output string
          out = ""
          for (i = 1; i <= n; i++) {
            if (i == 1) out = paths[i]
            else out = out " " paths[i]
          }
          # Store for final sorting by first path
          first_path[hash] = paths[1]
          result[first_path[hash]] = out
        }
      }
      # Sort the results by the first path key
      n = asorti(result, sorted_keys)
      for (i = 1; i <= n; i++) {
        print result[sorted_keys[i]]
      }
    }
  '
}