find_dupes() {
    dir="$1"
    find "$dir" -type f -exec md5 -r {} + | sort | awk -F '  ' '{print $1, $2}' | sort | uniq -w32 -D | sort -k2 | awk '
        BEGIN {prev=""; group=""}
        $1 != prev {
            if (group != "") print group;
            prev=$1; group=$2;
        } else {
            group = group " " $2;
        }
        END {if (group != "") print group}
    ' | awk '
        BEGIN {prev=""; group=""}
        $1 != prev {
            if (group != "") {
                if (split(group, arr, " ") >= 2) print group;
            }
            prev=$1; group=$0;
        } else {
            group = group " " $0;
        }
        END {if (group != "") { if (split(group, arr, " ") >= 2) print group; }}
    '
}