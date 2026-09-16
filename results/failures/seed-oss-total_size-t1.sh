total_size() {
	local dir="$1" sum=0
	find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
		sum=$((sum + $(stat -f %z "$file")))
	done
	printf '%d\n' "$sum"
}