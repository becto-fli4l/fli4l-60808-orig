#!/usr/bin/perl

use Net::Domain qw (hostname hostfqdn hostdomain);

$BOM = chr(0xFEFF);
$indent = 0;
$indent_char = "                                                                     ";

# when was src/packages/ introduced?
$MOVEREV = 29583;

sub do_indent {
    print substr($indent_char, 0, $indent * 2);
}

sub out {
    foreach (@_) {
	do_indent;
	print substr($indent_char, 0, $indent * 2) . $_ . "\n";
    }
}

sub open_ctxt {
    $indent++;
}
sub close_ctxt {
    $indent--;
}

require "$program_dir/../../../common/tex/doc/gen_changes_opts.pm";

if (@ARGV < 1 || @ARGV > 2) {
    print {STDERR} "usage:\n  gen_changes.pl old-version [new-version|-r]
    -r = add Last Changed Rev (trunk)\n";
    exit;
}

if (hostfqdn() eq "eisler.nettworks.org") {
	$url_prefix = "file:///repo";
}
else {
	$url_prefix = "https://repo.nettworks.org/svn";
}
$old_version = $ARGV[0];
$tag = $old_version;
$tag =~ s/\./_/g;
$tag = "fli4l-" . $tag;
$old_svnurl = "$url_prefix/fli4l/tags/$tag";

system ("svn list $old_svnurl >/dev/null 2>&1");
if ($?) {
    print {STDERR} "Error: old version $old_version is not available in the repository.\n";
    exit;
}

if (defined $ARGV[1] && (not $ARGV[1] =~ /-r/)) {
    $version = $tag = $ARGV[1];
    $tag =~ s/\./_/g;
    $tag = "fli4l-" . $tag;
    $new_svnurl = "$url_prefix/fli4l/tags/$tag";
} else {
    if (!open (VERFILE, "$program_dir/../../../../../base/version.txt")) {
	print {STDERR} "Error: cannot determine new version\n";  
	exit;
    }
    chomp ($version = <VERFILE>);
    ($branch = $version) =~ s/^(\d+\.\d+).*$/$1/;
    close (VERFILE);

    $new_svnurl = "$url_prefix/fli4l/branches/$branch/trunk";
}

system ("svn list $new_svnurl >/dev/null 2>&1");
if ($?) {
    print {STDERR} "Error: new version $version is not available in the repository.\n";
    exit;
}

open (CVS, "svn info --xml $old_svnurl|");
while($info = <CVS>) {
    next unless ($info =~ /<commit/);
    $info = <CVS>;
    chomp($oldRev = $info);
    $oldRev =~ s/.*revision="(.*)".*/$1/;
}
close(CVS);
open (CVS, "svn info --xml $new_svnurl|");
while($info = <CVS>) {
    next unless ($info =~ /<commit/);
    $info = <CVS>;
    chomp($newRev = $info);
    $newRev =~ s/.*revision="(.*)".*/$1/;
}
close(CVS);
if ($oldRev > $newRev) {
    print {STDERR} "Error: 'new' version (Rev.$newRev) is older than 'old' version (Rev.$oldRev).\n";
    exit;
}
if ($oldRev =~ $newRev) {
    print {STDERR} "Error: new version and old version are the same (Rev.$oldRev).\n";
    exit;
}

if (defined $ARGV[1] && ($ARGV[1] =~ /-r/)) {
    # -r use revision
    $version = $version . "-rev" . $newRev;
}

#
# lookup check file names
#
foreach $d (@dir) {
    $subdir = $d . "/check/";
    open(CVS, "svn list $new_svnurl/src/packages/$subdir 2>/dev/null|");
    while ($name = <CVS>) {
	if ( $name =~ /\.txt$/ ) {
	    push (@files, $subdir . $name) unless ($name =~ /mkfli4l\.txt/ || $name =~ /crlf-test.txt/) ;
	}
    }
    close (CVS);
}

out('\section{' . "$_GC_differences $version $_GC_and $old_version" . '} {\scriptsize \begin{longtable}{@{}p{8cm}p{8cm}@{}}');
open_ctxt;

foreach $file (@files) {
    %cur_var = ();
    @cur_var = ();
    %old_var = ();
    @old_var = ();

    #
    # get currently defined variables
    #
    open (CVS, "-|:utf8", "svn cat $new_svnurl/src/packages/$file");
    while (<CVS>) {
	s/^$BOM//;
	next if (/^#.*/);
	if (/^(\s|[+])*(\S+).*/) {
	    $var = $2;
	    next if ($var =~ /.*_DO_DEBUG$/);
	    $var =~ s/%/x/g;
	    push (@cur_var, $var);
	    $cur_var{$var} = 1;
	}
    }
    close (CVS);

    #
    # get variables defined in old version
    #
    open (CVS, "-|:utf8", "svn cat -r $oldRev $new_svnurl/src/packages/$file@$MOVEREV") || die "can't open  $old_svnurl/$file";
    while (<CVS>) {
	s/^$BOM//;
	next if (/^#.*/);
	if (/^(\s|[\+])*(\S+).*/) {
	    $var = $2;
	    next if ($var =~ /.*_DO_DEBUG$/);
	    $var =~ s/%/x/g;
	    push (@old_var, $var);
	    $old_var{$var} = 1;
	}
    }
    close (CVS);

    #
    # get deleted variables (variables present in the old release, but not
    # in the new one)
    #
    %deleted_variables = ();
    foreach $var (@old_var) {
	$deleted_variables{$var} = 1 unless defined ($cur_var{$var});
    }

    #
    # get new variables (variables present in the new release, but not
    # in the old one)
    #
    %new_variables = ();
    foreach $var (@cur_var) {
	$new_variables{$var} = 1 unless defined ($old_var{$var}) || defined($ignored{$var});
    }

    #
    # get renamed variables and remove them from new and deleted hashes
    #
    %renamed_variables = ();
    foreach $var (keys %deleted_variables) {
	if (defined $alias{$var}) {
	    $renamed_variables{$var} = $alias{$var};
	}
    }
    foreach $var (keys %renamed_variables) {
	delete $deleted_variables{$var};
	$new = $renamed_variables{$var};
	@newvars = split(/:/, $new);
	foreach $newvar (@newvars) {
	    delete $new_variables{$newvar};
	}
    }

    $packet_header = 0;
    @renamed_variables = sort keys(%renamed_variables);
    @new_variables = sort keys(%new_variables);
    @deleted_variables = sort keys(%deleted_variables);

    next unless $#renamed_variables >= 0 || $#new_variables >= 0 || $#deleted_variables >= 0;

    $package = `basename $file`;
    chomp ($package);
    $package =~ s/\.txt//;
    $package =~ s/_/\\_/g;
    $package =~ tr /[a-z]/[A-Z]/;

    #
    # package header
    #
    out('% package ' . $package);
    out ('\multicolumn{2}{p{12cm}}{\begin{center}{\Large \bfseries ' . "$_GC_package $package" . '}\end{center}} \\\\');
    open_ctxt;
    #
    # dump renamed variables
    #
    if ($#renamed_variables >= 0) {
	#
	# column header
	#
	print "% renamed: ".join(" ", @renamed_variables)."\n";
	out ('\multicolumn{2}{p{12cm}}{\begin{center}{\large \bfseries ' . $_GC_renamed_vars . '}\end{center}}\\\\',
	     '{\normalsize \bfseries ' . $_GC_original_name . '} & {\normalsize \bfseries ' . $_GC_new_name . ' } \\\\');
	open_ctxt;

	foreach $var (@renamed_variables) {
	    $labels = '';
	    $new = $renamed_variables{$var};
	    $vari = $var;
	    $vari =~ s/_/\\_/g;
	    $var   =~ s/_/\\_\\-/g;
	    @newvars = split(/:/, $new);
	    foreach $newvar (@newvars) {
		$label  = $newvar;
		$label  =~ s/_//g;
		$newvar =~ s/_/\\_\\-/g;
		$labels = $labels . " \\jump{$label}{$newvar}";
	    }
	    out ("\\index{$vari}$var & $labels \\\\");
	}
	out ("&\\\\");               # empty row
	close_ctxt;
    }

    #
    # dump new and removed variables
    #
    $max_index = $#new_variables > $#deleted_variables ? $#new_variables : $#deleted_variables;
    if ($max_index >= 0) {
	print "% deleted: ".join(" ", @deleted_variables)."\n";
	out "{\\begin{center}{\\large \\bfseries $_GC_new_vars}\\end{center}} & {\\begin{center}{\\large \\bfseries $_GC_obsolete_vars}\\end{center}} \\\\";
	open_ctxt;
	for ($i = 0; $i <= $max_index; $i++) {
	    $new = $del = '';
	    if (defined $new_variables[$i]) {
		$label = $new = $new_variables[$i];
		$new   =~ s/_/\\_\\-/g;
		$label =~ s/_//g;
		$new = "\\jump{$label}{$new}";
	    }
	    if (defined $deleted_variables[$i]) {
		$del = $deleted_variables[$i];
		$deli = $del;
		$deli =~ s/_/\\_/g;
		$del =~ s/_/\\_\\-/g;
		$del = "\\index{$deli}$del";
	    }
	    out ($new . ' & ' . $del . '  \\\\');
	}
	close_ctxt;
    }
    close_ctxt;
}
close_ctxt;
out '\end{longtable}}';
return 1;
