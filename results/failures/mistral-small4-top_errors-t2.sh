top_errors() {
  grep -o 'ERROR: [^ ]*' "$1" | sed 's/ERROR: //; s/[0-9]\+/#/g' | sort | uniq -c | sort -nr | sort -k2 | awk '{count=$1; $1=""; print count " " $0}' | sed 's/^ //; s/ $//'
}