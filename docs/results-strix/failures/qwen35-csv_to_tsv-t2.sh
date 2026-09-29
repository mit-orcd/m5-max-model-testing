csv_to_tsv() {
  awk -v RS='(\r?\n)?"?\r?\n|"\r?\n' -v ORS='' '
  BEGIN { FS=""; OFS="" }
  {
    # Remove leading/trailing whitespace around the record if needed, 
    # but RFC 4180 handles fields strictly.
    # We need to parse the record manually to handle quoted fields.
    
    # The RS handling is tricky for multiline fields. 
    # Let's switch to a character-by-character approach or a robust state machine in awk.
    # Since standard awk is limited, we will use a state machine loop.
    
    # Actually, let's use a simpler approach: read the whole file into a buffer or process line by line
    # but handle multiline fields explicitly.
    
    # Re-implementing with a more robust parsing logic in awk:
    # We will read the input char by char using "getline" with a dummy RS to simulate it,
    # but that is slow. Instead, we will read lines and manage state.
    
    # Let's reset for a cleaner logic:
    # We will accumulate the current record in a variable `rec`.
    # We track if we are inside quotes.
  }
  ' "$1" 2>/dev/null

  # The above awk logic is incomplete for multiline. 
  # Let's write a pure bash solution using `read` or a robust awk script that handles the multiline state.
  # Given the constraints, a pure bash loop is safer for multiline field handling with newlines.
  
  : # Placeholder to be replaced by the actual implementation below
}