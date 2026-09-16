find_dupes() {
  local dir="$1"
  local tmpfile=$(mktemp)

  # Find regular files, compute MD5, output "MD5 filename"
  find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | sort > "$tmpfile"

  # Group by MD5, collect paths, filter groups with >=2 files
  # Output format: one line per group with space-separated sorted paths
  # Groups sorted by first path

  awk '
  {
    # First field is MD5, rest is filename (may contain spaces)
    md5 = $1
    fname = substr($0, length($1) + 2)

    if (md5 != prev_md5) {
      if (count >= 2) {
        # Output previous group
        print group
      }
      group = fname
      count = 1
      prev_md5 = md5
    } else {
      group = group " " fname
      count++
    }
  }
  END {
    if (count >= 2) {
      print group
    }
  }
  ' "$tmpfile" | sort

  rm -f "$tmpfile"
}