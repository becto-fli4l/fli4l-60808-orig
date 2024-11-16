#! /usr/bin/perl -w 

use strict;
use warnings;

our(@ignore);

# $state=0;
# $header=0;
# $found=0;
# $first=1;

sub ignore_file
{
    my($name) = @_;
    foreach my $pat (@ignore) {
	return 1 if ($name =~ /$pat/);
    }
    return 0;
}

sub filter_bins
{
    my($state, $file, $first, @lines) = (0,"",1,()); 
    while (<>) 
    {
	if (/^Index:\s+(\S+)/) {
	    $state = 0;
	}

	if ($state == 0 || $state == 4) {
	    if (/^Index:\s+(\S+)/) {
		$file=$1;
		if (!ignore_file($file)) {
		    @lines = ();
		    push(@lines, $_);
		    $state=1;
		} else {
		    print "<A NAME=\"$file\"><H3>$file (Changes Ignored)</H3></A>\n";
		    $state=0;
		}
		next;
	    }
	    if ($state == 4) {
		s/&/&amp;/g;
		s/</\&lt;/g; 
		s/>/\&gt;/g;
		print;
	    }
	}
	
	if ($state == 1) {
	    if (/^====/) {
		push(@lines, $_);
		$state=2;
	    }
	    next;
	}
	
	if ($state == 2) {
	    if (/^[-+]/) {
		print "</pre>\n" unless $first;
		$first = 0;
		
		print "<A NAME=\"$file\"><H2>$file</H2></A>\n<pre>\n";
		print @lines;
		print;
		$state=4;
		next;
	    }
	    $state = 0;
	}
    }
    print "</pre>" unless $first;
}

sub grep_changes
{
    my($state, $found, $first, @lines) = (0,0,1,()); 
    while (<>) 
    {
	if ($state == 0 || $state == 4) {
	    if (/^Index:\s+(changes\/\S+)/) {
		@lines = ();
		push(@lines, $_);
		$state=1;
		next;
	    }
	    if (/^Index:\s+(\S+)/) {
		$state=0;
	    }
	    if ($state == 4) {
		print;
	    }
	}
	
	if ($state == 1) {
	    if (/^====/) {
		push(@lines, $_);
		$state=2;
	    }
	    next;
	}
	
	if ($state == 2) {
	    if (/^[-]/) {
		print "</pre>\n" unless $first;
		$first = 0;

		print "<pre>\n";
		print @lines;
		print;
		$state=4;
		$found=1;
		next;
	    }
	    $state = 0;
	}
    }
    print "</pre>" unless $first;

    if ($found) {
	exit 0;
    }
    else {
	exit 1;
    }
}

sub grep_bins 
{
    my($header, $found, $file) = (0,0, ""); 
    while (<>) {
	if ($header) {
	    if (/^[-]/) {
		$header=0;
		next;
	    }
	}
	if (/^Index:\s+(\S+)/) {
	    if ($header) {
		print "$file\n";
		$found=1;
	    }
	    $header=1;
	    $file=$1;
	}
    }

    if ($found) {
	exit 0;
    }
    else {
	exit 1;
    }
}

sub filter_stat
{
    my($stat, $prefix, $column_prefix) = ("", "", "");
    if (defined ($ARGV[0])) {
	$prefix = $ARGV[0];
	$column_prefix = "<td></td>";
	shift (@ARGV);
	# print "'$ARGV[0]'\n";
    }

    while (<>) {
	if (/.*file.*changed.*/) {
	    print "    <tr>$column_prefix<td colspan=4>$_</td></tr>\n";
	}
	elsif (/^\s*(\S+)[\s|]+(\S+)(\s*(\S+))?/) {
	    if (defined($4)) {
		$stat = $4;
	    }
	    else {
		$stat = "";
	    }
	    if (!ignore_file($1)) {
		print <<EOF;
        <tr>
	    $column_prefix
	    <td><A HREF="$prefix#$1">$1</A></td>
	    <td>$2</td>
	    <td>$stat</td>
	</tr>
EOF
            }
        }
    }
}

die "usage: filter_diff.pl <changes|bin|filter|stat> [ -x pattern[,pattern]]" if ($#ARGV < 0);
my($cmd) = $ARGV[0];
shift;

while ($#ARGV > 0) {
    if ($ARGV[0] eq "-x" && $#ARGV > 0) {
	push(@ignore, split(/,/ ,$ARGV[1]));
	shift;
	shift;
    } else {
	last;
    }
}

die "too many Arguments, usage: filter_diff.pl <changes|bin|filter|stat> [ -x pattern[,pattern]]" if ($#ARGV > 0);
if ($cmd eq "changes") {
    grep_changes;
}
if ($cmd eq "bin") {
    grep_bins;
}
if ($cmd eq "filter") {
    filter_bins;
}
if ($cmd eq "stat") {
    filter_stat;
}
