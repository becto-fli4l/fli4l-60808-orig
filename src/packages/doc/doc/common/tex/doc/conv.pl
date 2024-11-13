#! /usr/bin/perl -w
$debug = 0;
$open_description = 0;
$open_verbatim = 0;

sub end_description  {
  if ($open_description) {
    print "\\end{description}\n";
    $open_description = 0;
  }
}
sub begin_description  {
  if (! $open_description) {
    print "\\begin{description}\n";
    $open_description = 1;
  }
}
sub begin_verbatim  {
  if (! $open_verbatim) {
    print "\\begin{verbatim}\n";
    $open_verbatim = 1;
  }
}
sub end_verbatim  {
  if ( $open_verbatim) {
    print "\\end{verbatim}\n";
    $open_verbatim = 0;
  }
}

while ( <> ) {
  chomp ;
  if (/^(\s*)(OPT_[A-Z_0-9]+)\s*$/) {
    print "% match /^(\\s*)(OPT_[A-Z_0-9]+)\\s*\$/\n" if $debug;
    end_verbatim;
    end_description;
    $space = $1;
    $sec = $2;
    $label = $2;
    $label =~ s/_/-/g;
    $label =~ tr/A-Z/a-z/;
    $sec =~ s/_/\\_/g;
    print "\\marklabel{sec:$label}\n{\n\\section {$sec}\n}\\index{$sec}\n";
  } elsif (/^(\s*)([A-Z_0-9x%]+)\s*$/) {
    print "% matched /^(\\s*)([A-Z_0-9x%]+)\\s*\$/\n" if $debug;
    
    $space=$1;
    $var = $2;
    $label = $2;
    $var =~ s/_/\\_/g;
    $var =~ s/%/x/g;
    $label =~ s/_//g;
    $label =~ s/%/x/g;
    
    end_verbatim;
    begin_description;
#    print "$space";
    print "\\config";
    print "{$var}{$var}";
    print "{$label}\n";
  } elsif (/^(\s*)([A-Z_0-9x%]+\s+[A-Z_0-9x%]+).*\$/) {
    print "% matched /^(\\s*)([A-Z_0-9x%]+\\s+[A-Z_0-9x%]+).*$/\n" if $debug;
    end_verbatim;
    begin_description;
    $space = $1;
    $config = $2;
    @config = split (/\s+/, $config);
    foreach $var (@config) {
      $label = $var;
      $var =~ s/_/\\_/g;      
      $var =~ s/%/x/g;
      $label =~ s/_//g;
      $label =~ s/%/x/g;
      print "\\configlabel{$var}{$label}\n"
    } 
    $config =~ s/_/\\_/g;
    $config =~ s/%/x/g;
    print "\\configvar {$config}{$config}\n";
  } elsif (/^(\s*)([A-Z_0-9x]+\s*=.*)$/) {
    print "% matched /^(\\s*)([A-Z_0-9x]+\\s*=.*)\$/\n" if $debug;
    begin_verbatim;
    print "\t$2\n";
  } else {
    print "% matched  else" if $debug;
    end_verbatim;
    s/_/\\_/g;
    if (/^(.*)((ht|f)tp:\/\/[-\w\/.\\~]+)(.*)$/) {
      print " - link-check matches\n" if $debug;
      print "$1\\altlink{$2}$4\n";
    }
    else {
      print " - link-check did not match\n" if $debug;
      print "$_\n";
    }
  }
}
end_verbatim;
end_description;
