csv_to_tsv() {
	tr -d '\r' | awk '
BEGIN {
	output_has_lines = 0
}
{
	output_has_lines = 1
	line = $0
	nf = 0
	in_quotes = 0
	field = ""
	len = length(line)
	i = 1
	while (i <= len) {
		c = substr(line, i, 1)
		if (in_quotes) {
			if (c == "\"") {
				if (i+1 <= len && substr(line, i+1, 1) == "\"") {
					field = field "\""
					i += 2
				} else {
					in_quotes = 0
					i += 1
				}
			} else {
				field = field c
				i += 1
			}
		} else {
			if (c == "\"") {
				in_quotes = 1
				i += 1
			} else if (c == ",") {
				fields[++nf] = field
				field = ""
				i += 1
			} else {
				field = field c
				i += 1
			}
		}
	}
	fields[++nf] = field

	for (f = 1; f <= nf; f++) {
		processed = fields[f]
		if (length(processed) >= 2 && substr(processed, 1, 1) == "\"" && substr(processed, length(processed), 1) == "\"") {
			processed = substr(processed, 2, length(processed) - 2)
			gsub(/""/, "\"", processed)
		}
		gsub(/\t/, "\\\\t", processed)
		gsub(/\n/, "\\\\n", processed)
		pf[f] = processed
	}

	for (f = 1; f <= nf; f++) {
		if (f > 1) printf "\t"
		printf "%s", pf[f]
	}
	printf "\n"
}
END {
	if (!output_has_lines) print ""
}
'
}