function csv_to_tsv() {
    tr -d '\r' < "$1" |
    sed -E '
        s/^""//;
        s/""$/;
        s/""/"/g;
        s/^"/"(;
        s/"$/";
        s/"/,|\n|^$|^"/: "\\\1";
    ' |
    tr ',' '\t' |
    sed -E 's/\t+/\t/g;s/^[^\t]*\t//;s/\t[^\t]*$//'
}