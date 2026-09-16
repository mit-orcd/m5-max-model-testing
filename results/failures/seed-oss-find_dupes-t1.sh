find_dupes() {
	local dir="$1"
	find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
		md5 -r "$file" | awk '{print $1}'
		echo "$file"
	done | sort -z | uniq -z -d | xargs -0 | while IFS= read -r -d $'\n' hash; do
		echo "$hash" | tr ' ' '\n' | sort | tr '\n' ' ' | sed 's/ $//'
	done
}