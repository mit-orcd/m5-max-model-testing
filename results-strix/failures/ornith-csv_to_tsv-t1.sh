csv_to_tsv() {
  perl -Mstrict -Mwarnings -e '
    binmode(STDIN);
    binmode(STDOUT);
    my $file = $ARGV[0];
    open(my $fh, "<", $file) or die "Cannot open $file: $!";
    local $/;
    my $data = <$fh>;
    close($fh);
    $data =~ s/\r\n/\n/g;
    $data =~ s/\r/\n/g;
    my @out;
    my $i = 0;
    my $n = length($data);
    while ($i <= $n) {
      my @fields;
      while (1) {
        my $field = "";
        if ($i < $n && substr($data, $i, 1) eq "\"") {
          $i++;
          while ($i < $n) {
            my $c = substr($data, $i, 1);
            if ($c eq "\"") {
              if ($i + 1 < $n && substr($data, $i + 1, 1) eq "\"") {
                $field .= "\"";
                $i += 2;
              } else {
                $i++;
                last;
              }
            } else {
              $field .= $c;
              $i++;
            }
          }
        } else {
          while ($i < $n) {
            my $c = substr($data, $i, 1);
            if ($c eq "," || $c eq "\n") {
              last;
            }
            $field .= $c;
            $i++;
          }
        }
        $field =~ s/\t/\\t/g;
        $field =~ s/\n/\\n/g;
        push @fields, $field;
        if ($i < $n && substr($data, $i, 1) eq ",") {
          $i++;
          next;
        }
        if ($i < $n && substr($data, $i, 1) eq "\n") {
          $i++;
        }
        push @out, join("\t", @fields);
        last;
      }
      last if $i > $n;
    }
    print join("\n", @out), "\n";
  ' "$1"
}