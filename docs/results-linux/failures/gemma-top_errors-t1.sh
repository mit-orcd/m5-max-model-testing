top_errors() {
    local logfile="$1"
    local n="$2"

    if [[ ! -f "$logfile" ]]; then
        return
    fi

    grep "ERROR: " "$logfile" | \
    sed 's/.*ERROR: //' | \
    sed -E 's/[0-9]+/#/g' | \
    sort | \
    uniq -c | \
    sort -k1,1nr -k2,2 | \
    head -n "$n" | \
    awk '{count=$1; $1=""; print count substr($0,2)}' | \
    awk '{print $1 " " substr($0, index($0,$2))}' | \
    sed -E 's/^([0-9]+) (.*)/\1 \2/' | \
    while read -r count sig; do
        echo "$count $sig"
    done | \
    awk '{
        count=$1; 
        $1=""; 
        sub(/^ /, "", $0); 
        print count " " $0
    }' | \
    sort -k1,1nr -k2,2 | \
    head -n "$n" | \
    awk '{
        line=$0;
        split(line, a);
        count=a[1];
        sig=substr(line, index(line, a[2]));
        print count " " sig
    }' | \
    sed -E 's/^([0-9]+) (.*)/\1 \2/' | \
    awk 'BEGIN{OFS=" "} {count=$1; $1=""; sub(/^ /, "", $0); print count, $0}' | \
    sort -k1,1nr -k2,2 | \
    head -n "$n"
}