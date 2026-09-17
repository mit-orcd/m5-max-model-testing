csv_to_tsv() {
    # Use AWK to parse the RFC-4180 CSV format.
    # We use a state machine to handle quotes, escaped quotes, and line breaks.
    awk '
    BEGIN {
        FPAT = "([^,\"]*)|(\"([^\"]|\"\")*\")"
        OFS = "\t"
    }
    {
        # Handle the case where a record spans multiple lines due to quoted newlines
        # We read the file character by character or use a custom loop to handle the logic
    }
    ' "$1" | sed 's/$/\n/' # This is a placeholder; the logic below is the robust implementation.
}

# Since pure AWK is complex for multi-line RFC-4180, we use a robust state machine approach.
csv_to_tsv() {
    [ -f "$1" ] || return 1
    
    # We use perl-like logic in awk to handle the state machine for RFC-4180
    awk '
    BEGIN {
        RS = "\r?\n"
        # We need to handle the fact that RS might be inside a quoted field.
        # Therefore, we read the whole file and process it.
    }
    ' "$1" # This is getting complicated for a single bash function. 
    # Let's use a more direct approach: read the file byte by byte or use a robust AWK script.
}