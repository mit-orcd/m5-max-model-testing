function find_dupes() {
    local dir="$1"
    local md5s=$(md5 -r "$dir" | awk '{print $1, $2}')
    local groups=$(echo "$md5s" | sort -k2 | uniq -w32 -c -i | awk '!seen[$2]++ {print $2, $1}')
    local paths=$(echo "$groups" | awk '{print $2}' | xargs -I{} find "{}" -type f -print0 | sort -z | tr '\0' '\n')
    local result=$(echo "$paths" | xargs -I{} sh -c 'echo "$(md5 -r "{}") {}"')
    echo "$result" | awk -F'[ \t]+' 'NR==FNR{a[$1]=$2;next}{if(a[$1]!="")print a[$1],$2}' FS='\n' RS=
}