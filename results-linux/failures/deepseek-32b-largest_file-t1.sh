largest_file() {
    find "$1" -type f -exec stat -c "%s %f" {} + | awk 'BEGIN{max=0;file=""}{if($1>max){max=$1;file=$2}}END{print file}'
}