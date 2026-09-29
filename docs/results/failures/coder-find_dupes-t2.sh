find_dupes() {
    local dir="$1"
    local temp_file="$HOME/.find_dupes_temp"
    
    find "$dir" -type f -exec md5 -r {} \; | awk '{print $1}' | sort | uniq -d | while read -r hash; do
        find "$dir" -type f -exec md5 -r {} \; | grep "$hash" | awk '{print $2}' | sort | tr '\n' ' ' | sed 's/ *$//' 
done | sort | awk '{for(i=1;i<=NF;i++) if($i != "") {print $i}}' | sort | awk '{if($0 in seen) {seen[$0] = seen[$0] " " $0} else {seen[$0] = $0}} END {for (file in seen) {if (seen[file] != "") {print seen[file]}}}' | sort | awk '{if (NF > 1) {for(i=1;i<=NF;i++) {printf "%s ", $i} print ""}'
}