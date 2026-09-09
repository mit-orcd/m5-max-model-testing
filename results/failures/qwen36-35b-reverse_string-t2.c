void reverse_string(char *s) {
	if (!s)
		return;
	size_t len = 0;
	while (s[len] != '\0')
		len++;
	size_t i = 0;
	size_t j = len - 1;
	while (i < j) {
		char tmp = s[i];
		s[i] = s[j];
		s[j] = tmp;
		i++;
		j--;
	}
}