find_dupes() {
  local dir="$1"
  find "$dir" -type f -exec md5 -r {} + | sort | awk '{
    md5=$1
    path=$2
    if (md5 != prev) {
      if (NR > 1) {
        if (count > 1) {
          for (i=1; i<=count; i++) print paths[i]
        }
        delete paths
      }
      count=1
      paths[count]=path
      prev=md5
    } else {
      count++
      paths[count]=path
    }
  } END {
    if (count > 1) {
      for (i=1; i<=count; i++) print paths[i]
    }
  }' | sort | uniq -f1 -d
}