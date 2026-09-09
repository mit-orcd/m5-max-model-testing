function extract_urls() {
    while IFS= read -r line; do
        echo "https://$(echo $line | sed -n 's/^.*https:\/\//https:\/\//p')https://$(echo $line | sed -n 's/^.*http:\/\//http:\/\//p')"
    done
}