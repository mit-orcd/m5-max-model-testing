csv_to_tsv() {
  local file="$1"
  local ch=""
  local state="unquoted"
  local in_quote="0"
  local doubled="0"
  local field=""
  local record=""
  local had_char="0"

  while IFS= read -r -n1 ch; do
    [[ "$ch" == *$'\r'* ]] && continue
    [[ -z "$ch" ]] && break

    had_char="1"

    case "$state" in
      quoted)
        case "$ch" in
          '"')
            if [[ "$doubled" == "1" ]]; then
              field+='"'
              doubled="0"
            else
              in_quote="0"
              state="after_quote"
            fi
            ;;
          $'\t')
            field+='\\t'
            ;;
          $'\n')
            field+='\\n'
            ;;
          *)
            field+="$ch"
            ;;
        esac
        ;;
      after_quote)
        case "$ch" in
          ',')
            record+="$field"
            record+=$'\t'
            field=""
            state="unquoted"
            ;;
          $'\n')
            record+="${field%$'\t'}"
            printf '%s\n' "${record%$'\t'}"
            record=""
            field=""
            state="unquoted"
            ;;
          *)
            field+="$ch"
            state="unquoted"
            ;;
        esac
        ;;
      unquoted)
        case "$ch" in
          ',')
            record+="$field"
            record+=$'\t'
            field=""
            ;;
          $'\n')
            record+="${field%$'\t'}"
            printf '%s\n' "${record%$'\t'}"
            record=""
            field=""
            ;;
          '"')
            field+='"'
            in_quote="1"
            state="quoted"
            doubled="0"
            ;;
          $'\t')
            field+='\\t'
            ;;
          *)
            field+="$ch"
            ;;
        esac
        ;;
    esac
  done < "$file"

  if [[ "$had_char" == "1" ]]; then
    record+="${field%$'\t'}"
    printf '%s\n' "${record%$'\t'}"
  fi
}