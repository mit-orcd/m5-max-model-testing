void trim(char *s) {
	if (!s) return;

	// Skip leading whitespace
	char *start = s;
	while (*start == ' ' || *start == '\t' || *start == '\n') {
		start++;
	}

	// If string is all whitespace or empty
	if (*start == '\0') {
		*s = '\0';
		return;
	}

	// Find end of string (last character before null terminator)
	char *end = start + __builtin_strlen(start) - 1;

	// Strip trailing whitespace
	while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
		end--;
	}

	// Move the trimmed content to the beginning of the original string
	*(end + 1) = '\0';
	__builtin_memmove(s, start, (size_t)(end - start + 2));
}