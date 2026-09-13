csv_to_tsv() {
    tr -d '\r' < "$1" |
        sed 's/"//g;s/^"//;s/"$/"' |
        tr -s '\t' '\n' |
        tr -s '\n' '\t' |
        sed 's/\t/"\t"/;s/"\t"/\t/g'
}