csv_to_tsv() {
  local file=$1
  local line
  local in_quote=false
  local current_field=
  local current_record=
  local IFS=$'\n'

  while IFS= read -r line; do
    line=${line%$'\r'}
    while true; do
      if $in_quote; then
        if [[ $line == *\"* ]]; then
          local part=${line%%\"*}
          current_field+=$part
          line=${line#*\"}
          in_quote=false
        else
          current_field+=$line
          break
        fi
      else
        if [[ $line == *\"* ]]; then
          local part=${line%%\"*}
          if [[ $part == *','* ]]; then
            current_record+=$current_field
            current_record+=$(echo -ne '\t')
            current_field=
            line=${line#*\",}
            in_quote=true
          else
            current_field+=$part
            line=${line#*\"}
            in_quote=true
          fi
        elif [[ $line == *','* ]]; then
          current_record+=$current_field
          current_record+=$(echo -ne '\t')
          current_field=
          line=${line#*,}
        else
          current_field+=$line
          break
        fi
      fi
      if [[ $line == *$'\n'* ]]; then
        line=${line%$'\n'}
        current_record+=$current_field
        current_record=$(echo -e "${current_record}" | sed 's/"/"/g; s/\\\\/\\\\\\\\/g; s/\t/\\t/g; s/\n/\\n/g')
        echo "$current_record"
        current_record=
        current_field=
      fi
    done
    if [[ $line == *$'\n'* ]]; then
      line=${line%$'\n'}
    fi
    if [[ -n $line ]]; then
      current_record+=$current_field
      current_record=$(echo -e "${current_record}" | sed 's/"/"/g; s/\\\\/\\\\\\\\/g; s/\t/\\t/g; s/\n/\\n/g')
      echo "$current_record"
      current_record=
      current_field=
    fi
  done < "$file"
  if [[ -n $current_record ]]; then
    current_record=$(echo -e "${current_record}" | sed 's/"/"/g; s/\\\\/\\\\\\\\/g; s/\t/\\t/g; s/\n/\\n/g')
    echo "$current_record"
  fi
}