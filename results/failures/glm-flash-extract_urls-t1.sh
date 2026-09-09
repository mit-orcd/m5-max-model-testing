extract_urls() {
  awk -v RS="[[:space:]]+" '
  {
    if ($0 ~ /^https?:\/\/[\/a-z.\/?%&=\-#_+]*/) {
      print $0;
    }
  }
  '
}