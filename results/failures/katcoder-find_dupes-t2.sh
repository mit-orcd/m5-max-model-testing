find_dupes() {
	local dir="$1"
	declare -A seen
	while IFS= read -r -d '' line; do
		local hash="${line%% *}"
		local file="${line#* }"
		if [[ -z "${seen[$hash]}" ]]; then
			seen[$hash]="$file"
		else
			seen[$hash]="${seen[$hash]}"$'\n'"$file"
		fi
	done < <(find "$dir" -type f -exec md5 -r {} \; 2>/dev/null)
	local -a groups=()
	for hash in "${!seen[@]}"; do
		local val="${seen[$hash]}"
		if [[ "$val" == *$'\n'* ]]; then
			local sorted
			sorted=$(printf '%s\n' "$val" | sort)
			local joined
			joined=$(printf '%s ' $sorted)
			joined="${joined% }"
			groups+=("$joined")
		fi
	done
	IFS=$'\n' echo "${groups[@]}" | sort
}