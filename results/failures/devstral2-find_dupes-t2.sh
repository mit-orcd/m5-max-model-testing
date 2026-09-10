find_dupes() {
    dir="$1"
    if [[ ! -d "$dir" ]]; then
        return 1
    fi

    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT

    find "$dir" -type f -exec md5 -r {} + | awk -F' = ' '{print $2, $1}' > "$tmpfile"

    awk -F'\t' '
        {hash=$1; file=$2; if(!(hash in hashes)){hashes[hash]=""; count[hash]=0} count[hash]++; files[hash,count[hash]]=file}
        END{
            for(hash in count){
                if(count[hash] >= 2){
                    printf("%s\t", hash);
                    for(i=1; i<=count[hash]; i++){printf("%s ", files[hash,i])}
                    printf("\n")
                }
            }
        }' "$tmpfile" | sort -k2 | awk -F'\t' '
        {
            hash=$1
            for(i=2; i<=NF; i++){files[hash,++count[hash]]=$i}
        }
        END{
            for(hash in count){
                if(count[hash] >= 2){
                    printf("%s", files[hash,1])
                    for(i=2; i<=count[hash]; i++){printf(" %s", files[hash,i])}
                    printf("\n")
                }
            }
        }' | sort
}