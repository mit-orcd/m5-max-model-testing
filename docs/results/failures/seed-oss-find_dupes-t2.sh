find_dupes() {
	local dir="$1"
	find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
		md5 -r "$file" | awk '{print $1}'
		echo -n "$file"$'\0'
	done | sort -z | while IFS= read -r -d '' line; do
		[[ $line =~ ^[0-9a-f]{32}$ ]] && hash="$line" && files=() || files+=("$line")
	done | {
		local hash files
		while IFS= read -r -d '' line; do
			if [[ $line =~ ^[0-9a-f]{32}$ ]]; then
				[[ ${#files[@]} -ge 2 ]] && IFS=$' ' echo "${files[*]}"
				hash="$line"
				files=()
			else
				files+=("$line")
			fi
		done
		[[ ${#files[@]} -ge 2 ]] && IFS=$' ' echo "${files[*]}"
	} | sort
}