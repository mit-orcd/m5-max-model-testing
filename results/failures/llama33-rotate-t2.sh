rotate() {
  local file=$1
  local num=1
  while [ -f "${file}.${num}" ]; do
    mv "${file}.${num}" "${file}.$((num + 1))"
    ((num++))
  done
  mv "${file}" "${file}.1"
}