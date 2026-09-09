largest_file() {
    find "$1" -type f -print0 | xargs -0 ls -l | awk '{print $9}' | xargs -I {} stat -c "%s {}" {} | awk '{if ($1 > max) {max=$1; file=$2}} END {print file}'
}