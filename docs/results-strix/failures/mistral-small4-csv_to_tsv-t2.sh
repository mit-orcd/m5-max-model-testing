csv_to_tsv() {
  local file="$1"
  local line
  local field
  local in_quotes=false
  local output_line=""

  while IFS= read -r line; do
    local field_count=0
    local record=()

    while IFS=',' read -r field; do
      if [[ "$in_quotes" == true ]]; then
        field_count=$((field_count + 1))
        record+=("$field")
      else
        if [[ "$field" == *\"* ]]; then
          if [[ "$field" =~ ^\"(.*)\"$ ]]; then
            field="${BASH_REMATCH[1]}"
            field="${field//\"\"/\"}"
            field_count=$((field_count + 1))
            record+=("$field")
          else
            in_quotes=true
            field_count=$((field_count + 1))
            record+=("$field")
          fi
        else
          field_count=$((field_count + 1))
          record+=("$field")
        fi
      fi
    done <<< "${line//$'\r'/$'\n'}"

    if [[ "$in_quotes" == true ]]; then
      local joined_field
      joined_field=$(printf "%s" "${record[@]}")
      field="${joined_field//$'\n'/\\n}"
      field="${field//$'\t'/\\t}"
      output_line+="$field"
      in_quotes=false
    else
      output_line+=$(printf "%s\t" "${record[@]}")
      output_line=${output_line%?}
    fi

    if [[ "$in_quotes" == false ]]; then
      echo "$output_line"
      output_line=""
    else
      output_line="$field"
    fi
  done < "$file"

  if [[ -n "$output_line" ]]; then
    echo "$output_line"
  fi
}