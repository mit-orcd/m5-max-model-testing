json_get() {
  python3 -c 'import sys, json; print(json.load(sys.stdin)[sys.argv[1]])'
}