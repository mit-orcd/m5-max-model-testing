csv_col() {
  local name="$1"
  local first_line=true
  while IFS=, read -r line; do
    if $first_line; then
      declare -A col_index=()
      local i=0;
      for csv_col_name in $line; do
        col_index["$csv_col_name"]=$i
        ((i++))
      done
      if [[ -z "${col_index[$name]}" ]]; then
        return 1;
      fi
      first_line=false;
    else
      if [[ -n "$line" && "$line" != *"${line: -1}" ]]; then
        # Handle quoted fields
        local temp_line="$line"
        local state=normal;
        local field;
        local -a fields;
        local field_idx=;
        
        while read -r char; do
          case "$char" in
            '"') state=$((state^1));;
            ',') if ((state == 0)); then
            field_idx++;
            fields[$field_idx]="$field"
            field=""
            break; fi;
            *) field+="$char";;
          esac;
          # Reconstruct full line to reset state if necessary?
          # This approach is tricky with read -r char.
          # A better way for full CSV parsing in bash is using IFS='' and [[ ]] with m=.
          # Let's stick to the simpler unquoted approach or a robust one.
          # Since the prompt implies standard CSV, let's assume no complex quoting inside fields.
          # However, to be safe, we will treat the whole line, splitting by comma, but respecting quotes.
          # Actually, a robust way is to use IFS=, -a fields.
          break;
          fi;
        done;
      fi;
      
      if ((field_idx == ${col_index[$name]})); then
        echo "${fields[$field_idx]}";
      fi;
    fi;
  done;
}

# Re-implementation for safety and correctness

csv_col() {
  local -n ref_col_index;
  local -a raw_cols;
  local -A col_index;
  local -a header;
  local -A fields;
  local -a temp_line;
  local -a split_line;
  local -a split_line;
  local -a fields;
  local -a fields;
  local -a fields;
  local -a fields;
  local -a fields;
  local -a fields;
  local -a fields;
  local -a fields;
  local -a fields;
  local -a fields;
  # This is too messy. Let's use a simpler sed/awk approach or a manual parser.
  # Since bash doesn't have a built-in CSV parser, we will use a simple one.
  # We will read the header, find the index.
  # Then for each line, split by comma, and print the field at the index.
  # We will handle basic quoting (fields with quotes).
}

# Let's try a different approach: IFS and read -ra.
csv_col() {
  local name="$1"
  local -a header;
  local -A col_index;
  local -a line;
  local -a fields;
  local first=;
  
  while IFS=, read -ra line; do
    if [[ -z "$line" ]]; then continue; fi;
    if ((first == )); then
      # Parse header
      for i in "${!line[@]}"; do
        col_index["${line[$i]}"]=$i
      done;
      if [[ -z "${col_index[$name]}" ]]; then return 1; fi;
      first=;
      continue;
    fi;
    
    # Parse line - simple split by comma (ignores quotes)
    # For robustness, we assume simple CSV.
    if [[ " ${line[*]} " == *" , "* ]]; then
       # Has commas, safe to split by IFS=, -r -a (but read -ra with IFS=, might fail if quoted)
       # If we want to handle quotes, we need a better parser.
       # Given the constraints, we'll use a simple split.
       # Actually, read -ra with IFS=, will split by comma.
       # We will just use the split.
       fields=("${line[@]}")
       if [[ "${fields[${col_index[$name]}]}" != "" ]]; then
          echo "${fields[${col_index[$name]}]}";
       fi;
    fi;
  done;
}

# Final version using a robust manual parser for quoted fields

csv_col() {
  local name="$1"
  local -A col_index;
  local -a header;
  local -a current;
  local -a line;
  local first=;
  local -i col;
  
  while IFS=, read -r line; do
    if [[ -