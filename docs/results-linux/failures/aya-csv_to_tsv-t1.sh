function csv_to_tsv () {
    local IFS=$'\n'
    local OFS=$'\t'
    local EFS=$'\n'
    local field
    local record=()
    while IFS= read -r line; do
        IFS=, OFS=$'\t' EFS=$'\n'
        read -r -a record <<< "$line"
        for field in "${record[@]}"; do
            # remove leading and trailing double quotes
            field=${field//^\""/}
            field=${field//"$"%}
            # escape tab and newline inside double quotes
            field=${field//\\t/\\t}
            field=${field//\\n/\\n}
            # unescape backslash-escaped double quotes
            field=${field//\\\"/\"}
        done
        echo "${record[*]}"
    done
}