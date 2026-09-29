find_dupes() {
  local dir="$1"
  [ -z "$dir" ] && return
  (
    cd -- "$dir" || exit
    find . -type f -print0 | while IFS= read -r -d '' f; do
      md5 -r "$f" | awk '{print $1}'
    done | paste -d '' - <(find . -type f -print0 | while IFS= read -r -d '' f; do printf '%s\n' "$f"; done) |
    sort -t$'\t' -k1,1 |
    awk -F$'\t' '
    {
      hash=$1; path=$2
      if (!(path in seen)) {
        seen[path]=1
        hashes[hash] = hashes[hash] ? hashes[hash] "\t" path : path
      }
    }
    END {
      n=0
      for (h in hashes) {
        gsub(/\t/, " ", hashes[h])
        groups[++n] = hashes[h]
      }
      # sort groups by first path
      for (i=1; i<=n; i++)
        for (j=i+1; j<=n; j++)
          if (groups[i] > groups[j]) {
            t=groups[i]; groups[i]=groups[j]; groups[j]=t
          }
      for (i=1; i<=n; i++) {
        split(groups[i], arr, " ")
        if (length(arr) >= 2) print groups[i]
      }
    }
    '
  )
}