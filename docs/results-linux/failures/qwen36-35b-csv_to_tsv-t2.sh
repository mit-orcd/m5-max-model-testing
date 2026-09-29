csv_to_tsv() {
    local file="${1:-/dev/stdin}"
    awk '
    BEGIN {
        FS = ""
        OFS = ""
        RS = "\n"
        in_quotes = 0
        field = ""
        first_field = 1
        record = ""
    }

    function process_char(c) {
        if (in_quotes) {
            if (c == "\"") {
                # Check for escaped quote ""
                # We need to look ahead, but awk is line-based/stream-based
                # Since we are processing char by char, we handle state differently
                # Actually, let us rethink: reading char by char in awk is hard.
                # Better approach: Read the whole record logic.
            }
        }
    }

    # Since pure awk char-by-char is tricky for RFC 4180 with newlines inside quotes,
    # we will use a different strategy:
    # 1. Read the entire file into a string, handling CRLF -> LF.
    # 2. Parse the string manually using a loop in awk.

    {
        # Append line to buffer, preserving newlines if they are part of the data
        # But wait, standard awk splits by RS. We need to handle the whole file.
        # Let's just accumulate all input into one variable.
        if (NR > 1) {
            buf = buf "\n" $0
        } else {
            buf = $0
        }
    }

    END {
        len = length(buf)
        i = 1
        output = ""
        
        while (i <= len) {
            # Start a new record
            fields_count = 0
            delete fields
            
            while (1) {
                c = substr(buf, i, 1)
                
                # Skip \r if present (should have been handled, but just in case)
                if (c == "\r") {
                    i++
                    continue
                }
                
                # If we are at the start of a field
                if (c == "\"") {
                    # Quoted field
                    field = ""
                    i++
                    while (i <= len) {
                        c = substr(buf, i, 1)
                        if (c == "\"") {
                            # Check next char
                            next_c = ""
                            if (i + 1 <= len) {
                                next_c = substr(buf, i + 1, 1)
                            }
                            if (next_c == "\"") {
                                # Escaped quote
                                field = field "\""
                                i += 2
                            } else {
                                # End of quoted field
                                i++
                                break
                            }
                        } else if (c == "\n") {
                            # Newline inside quoted field becomes \n literal
                            field = field "\\n"
                            i++
                        } else if (c == "\t") {
                            # Tab inside quoted field becomes \t literal
                            field = field "\\t"
                            i++
                        } else {
                            field = field c
                            i++
                        }
                    }
                    fields[fields_count] = field
                    fields_count++
                } else {
                    # Unquoted field
                    field = ""
                    while (i <= len) {
                        c = substr(buf, i, 1)
                        if (c == ",") {
                            i++
                            break
                        } else if (c == "\n") {
                            # End of record
                            # Check if it is CRLF (CR already skipped/ignored usually, but let's be safe)
                            # Actually, if we see \n here, it ends the record.
                            # But wait, what if the field ends with \r\n?
                            # We stripped \r in the loop above? No, we skipped \r.
                            # So if c is \n, record ends.
                            break
                        } else if (c == "\r") {
                            i++
                            continue
                        } else {
                            field = field c
                            i++
                        }
                    }
                    # Check if we hit a newline to end record
                    # If c is \n, we broke out. 
                    # Note: if the line ended with \r\n, and we skipped \r, we are at \n.
                    # The loop breaks on \n.
                    
                    fields[fields_count] = field
                    fields_count++
                    
                    # If we broke because of \n, we consume it
                    if (i <= len && substr(buf, i, 1) == "\n") {
                        i++
                        break
                    }
                    # If we broke because of , or end of string, we continue to next field
                    if (i > len) {
                        # End of input
                        break
                    }
                    if (substr(buf, i, 1) == ",") {
                        # Comma was consumed in the loop? 
                        # In unquoted loop: if c==",", i++, break.
                        # So comma is consumed.
                        # Continue to next field
                        continue
                    }
                    # If we broke because of other reasons (like end of string), handle exit
                    if (i > len) break
                }
                
                # Check for comma or end of record
                if (i <= len) {
                    c = substr(buf, i, 1)
                    if (c == ",") {
                        i++
                        continue
                    } else if (c == "\n") {
                        i++
                        break
                    } else if (c == "\r") {
                        i++
                        continue
                    } else {
                        # Unexpected character, should not happen in valid CSV
                        # But if it's something else, maybe end of record?
                        # Let's assume valid CSV.
                        break
                    }
                } else {
                    break
                }
            }
            
            # Join fields with tab
            line = ""
            for (j = 0; j < fields_count; j++) {
                if (j > 0) line = line "\t"
                line = line fields[j]
            }
            output = output line "\n"
        }
        
        printf "%s", output
    }
    ' "$file"
}