extract_urls() {
    grep -oE 'https?://[a-zA-Z0-9./_?=&%;+#:@~!$*\'()\'\[\]{}<>|\\,^-]+' 
}