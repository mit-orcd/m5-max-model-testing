find_dupes() {
  local dir="$1"
  find "$dir" -type f -exec md5 -r {} + | sort | awk '{
    md5=$1
    path=$2
    gsub(/^[ \t]+|[ \t]+$/, "", path)
    if (md5 in seen) {
      seen[md5] = seen[md5] " " path
    } else {
      seen[md5] = path
    }
  } END {
    for (m in seen) {
      if (split(seen[m], files) > 1) {
        print seen[m]
      }
    }
  }' | sort
}