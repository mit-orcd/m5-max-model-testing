extract_urls() {
	grep -o 'https?://[^[:space:]]*'
}