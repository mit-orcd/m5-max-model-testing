find_dupes() {
	local dir="${1:-.}" shopt -s nullglob globstar
	mapfile -t files < <(find "$dir" -type f)
	declare -A md5_map
	for file in "${files[@]}"; do
		md5=$(md5 -rq "$file")
		md5_map["$md5"]+="$file "
	done
	for md5 in "${!md5_map[@]}"; do
		paths=(${md5_map["$md5"]})
		(( ${#paths[@]} >= 2 )) || continue
		IFS=$' ' sorted=($(printf '%s\n' "${paths[@]}" | sort))
		echo "${sorted[*]}"
	done | sort -f
}