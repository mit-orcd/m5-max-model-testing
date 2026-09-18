csv_to_tsv() {
  local file="$1"
  local line field fields=() in_quoted=0 i c next_c
  local -a records
  local record=""

  while IFS= read -r line || [[ -n "$line" ]]; do
    # Normalize line endings: strip trailing CR if present
    line="${line%$'\r'}"
    fields=()
    in_quoted=0
    field=""
    i=0
    len=${#line}
    while (( i < len )); do
      c="${line:i:1}"
      next_c=""
      (( i+1 < len )) && next_c="${line:i+1:1}"

      if (( in_quoted )); then
        if [[ "$c" == '"' ]]; then
          if [[ "$next_c" == '"' ]]; then
            field+='"'
            (( i += 2 ))
            continue
          else
            in_quoted=0
            (( i++ ))
            continue
          fi
        else
          field+="$c"
          (( i++ ))
          continue
        fi
      else
        if [[ "$c" == '"' ]]; then
          in_quoted=1
          (( i++ ))
          continue
        elif [[ "$c" == ',' ]]; then
          fields+=( "$field" )
          field=""
          (( i++ ))
          continue
        else
          field+="$c"
          (( i++ ))
          continue
        fi
      fi
    done
    fields+=( "$field" )

    # Replace tab with \t, newline with \n, carriage return removed already
    for i in "${!fields[@]}"; do
      # Replace backslash first to avoid double escaping
      f="${fields[i]//\\/\\\\}"
      f="${f//$'\t'/\\t}"
      f="${f//$'\n'/\\n}"
      fields[i]="$f"
    done

    # Join with tab and output
    ( IFS=$'\t'; echo "${fields[*]}" )
  done < "$file"
  echo
}