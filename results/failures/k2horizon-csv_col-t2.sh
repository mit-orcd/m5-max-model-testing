csv_col() {
    local name="$1"
    local header line col
    IFS=',' read -r header || return 0
    col=0
    local IFS=','
    local prev
    prev=$(echo "$header" | awk -F',' '{for(i=1;i<=NF;i++) if($i==ENVIRON["name"]) print i; exit}' name="$name")
    [ -z "$prev" ] && return 1
    while IFS=',' read -r line; do
        [ -z "$line" ] && continue
        local IFS=','
        local -a fields=($line)
        echo "${fields[$prev]}"
    done
}