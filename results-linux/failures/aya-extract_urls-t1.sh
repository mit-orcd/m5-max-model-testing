function extract_urls() {
    perl -ne 'print "$1\n" while /(https?:\/\/[^\s]+)/g'
}