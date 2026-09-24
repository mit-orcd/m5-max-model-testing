csv_to_tsv() {
  local file="$1"
  local line
  local in_quotes=0
  local field=""
  local record=""
  local first_field=1

  while IFS= read -r line || [ -n "$line" ]; do
    # Strip trailing CR
    line="${line%$'\r'}"

    local i c
    local len=${#line}
    for (( i=0; i<len; i++ )); do
      c="${line:$i:1}"
      if [ "$in_quotes" -eq 1 ]; then
        if [ "$c" = '"' ]; then
          # Check for escaped quote
          if [ "${line:$((i+1)):1}" = '"' ]; then
            field+='"'
            i=$((i+1))
          else
            in_quotes=0
          fi
        elif [ "$c" = $'\t' ]; then
          field+=$'\\t'
        elif [ "$c" = $'\n' ]; then
          field+=$'\\n'
        else
          field+="$c"
        fi
      else
        case "$c" in
          '"')
            in_quotes=1
            ;;
          ',')
            if [ "$first_field" -eq 0 ]; then
              record+=$'\t'
            fi
            record+="$field"
            field=""
            first_field=0
            ;;
          $'\t')
            if [ "$first_field" -eq 0 ]; then
              record+=$'\t'
            fi
            record+="$field"
            field=""
            first_field=0
            ;;
          *)
            field+="$c"
            ;;
        esac
      fi
    done

    # End of line: emit field and record
    if [ "$first_field" -eq 0 ]; then
      record+=$'\t'
    fi
    record+="$field"
    printf '%s\n' "$record"
    record=""
    field=""
    first_field=1
  done < "$file"
}