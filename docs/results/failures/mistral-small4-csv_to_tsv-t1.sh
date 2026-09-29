csv_to_tsv() {
  local file="$1"
  local line
  local field
  local in_quotes=0
  local output_line=""

  while IFS= read -r line; do
    while IFS=, read -r -a fields || [ ${#fields[@]} -gt 0 ]; do
      if [ $in_quotes -eq 0 ]; then
        for field in "${fields[@]}"; do
          if [[ "$field" == *\"* ]]; then
            in_quotes=1
            field=${field#\"}
            field=${field%\"}
            field=${field//\"\"/\"}
            field=${field//'\n'/'\\n'}
            field=${field//$'\n'/'\\n'}
            field=${field//$'\t'/'\\t'}
          else
            field=${field//$'\n'/'\\n'}
            field=${field//$'\t'/'\\t'}
          fi
          output_line+=$'\t'"$field"
        done
        output_line=${output_line#$'\t'}
        printf '%s\n' "$output_line"
        output_line=""
        in_quotes=0
      else
        field=""
        for ((i=0; i<${#fields[@]}; i++)); do
          if [ $i -eq 0 ]; then
            output_line+="${fields[i]}"
          else
            output_line+=$'\t'"${fields[i]}"
          fi
        done
        in_quotes=0
      fi
    done < "$file"
  done
  printf '\n'
}