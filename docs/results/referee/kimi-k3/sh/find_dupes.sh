find_dupes() {
  find "$1" -type f -exec md5 -r {} + | sort | awk '
    { h=$1; $1=""; sub(/^ /, ""); files[h] = files[h] " " $0; cnt[h]++ }
    END { for (h in files) if (cnt[h] > 1) print substr(files[h], 2) }
  ' | sort
}
