find_dupes() {
  local dir=${1:-.}
  find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
    md5 -q "$file" | awk -v f="$file" '{print $0 "\t" f}'
  done | sort -k1,1 -k2,2 | awk '
  BEGIN{FS="\t"}
  {
    if ($1 != curhash) {
      if (n>=2) {
        printf "%s", path[1];
        for (i=2;i<=n;i++) printf " %s", path[i];
        printf "\n";
      }
      curhash=$1; n=1; path[1]=$2;
    } else {
      n++; path[n]=$2;
    }
  }
  END{
    if (n>=2) {
      printf "%s", path[1];
      for (i=2;i<=n;i++) printf " %s", path[i];
      printf "\n";
    }
  }' | sort -k1,1
}