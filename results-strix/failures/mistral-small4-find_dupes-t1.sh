find_dupes() {
  local dir="$1"
  local tmpfile=$(mktemp)
  find "$dir" -type f -exec md5 -r {} + | sort -k1,32 | awk '{
    md5 = $1;
    path = substr($0, index($0,$2));
    print md5, path
  }' | sort -k1 | awk '{
    if (NR == 1) {
      first_md5 = $1;
      first_path = $2;
    } else if ($1 != first_md5) {
      if (count > 1) print dupes;
      count = 0;
      delete dupes;
    }
    count++;
    dupes[count] = $2;
  }
  END {
    if (count > 1) print dupes
  }' | sed '/^$/d' | while read -r line; do
    count=$(echo "$line" | awk '{print NF}')
    if [ "$count" -ge 2 ]; then
      echo "$line" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//'
    fi
  done | sort -k2 | uniq -f1 | sed 's/^ *//'
  rm -f "$tmpfile"
}