find_dupes() {
	local dir="$1"
	find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
		md5 -r "$file" | awk '{print $1}'
		echo -n $'\0'
		echo "$file"
		echo -n $'\0'
	done | sort -z | uniq -zD | while IFS= read -r -d '' line; do
		echo "$line" | tr '\0' '\n' | sort | tr '\n' ' ' | sed 's/ $//'
	done
}