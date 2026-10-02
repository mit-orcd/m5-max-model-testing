extract_urls() {
    perl -nE 'while (/(https?:\/\/\S+)/g) { print "$1\n" }'
}