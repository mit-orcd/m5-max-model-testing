find_dupes() {
  local dir="$1"
  find "$dir" -type f -exec md5 -r {} + | sort | awk '{
    md5=$1
    path=$2
    if (md5 != prev) {
      if (NR > 1) {
        if (count > 1) {
          for (i=1; i<=NF; i++) if (i>1) print paths[i]
          print path
          for (i=1; i<=NF; i++) if (i>1) paths[i]=""
        }
        delete paths
        count=0
      }
      prev=md5
    }
    paths[++count]=$2
  } END {
    if (count > 1) {
      for (i=1; i<=NF; i++) if (i>1) print paths[i]
      print path
    }
  }' | sort -u
}