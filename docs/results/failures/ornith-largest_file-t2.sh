largest_file() {
  find "$1" -type f -printf '%s\t%p\n' 2>/dev/null \
    | sort -nrk1 \
    | awk -F'\t' 'NR==1{
        max=$1
        # print first matching line
        print $2
        exit
      }'
}