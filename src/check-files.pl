#! /usr/bin/perl
#------------------------------------------------------------------------------
# check-files.pl - show differences between files.txt and SVN tree
#
# usage: check-files.pl help
#
# Creation:    11.09.2003 jw5
# Last Update: $Id$
#------------------------------------------------------------------------------

# --- MODULES ---
use strict;
use warnings;
use File::Basename;
use File::Copy;
use Data::Dumper;
use feature 'say';

# --- CONSTANTS ---
use constant  LINELEN => 80;

# --- CONFIG ---
$ENV{'LC_ALL'} = 'C';

my $ARCH = $ENV{'ARCH'};
if (!defined($ARCH) || $ARCH eq "") {
    $ARCH = 'x86';
    $ENV{'ARCH'} = $ARCH;
}

# change directory to the directory containing this script
chdir(dirname($0));

# These are pre-compiled pattern, add new ones the same way
my @ignore = (
    qr{\.svn},
    qr{packages/[[:alnum:]]*/doc/[[:alnum:]]*/html},
    qr{.+~$},
    qr{.DS_Store$},
    );

# These is the fileslist where all files should be listed...
my $filelist = "files.txt";

# --- GLOBAL VARS ---
our (%real_files, %real_nonlink_files, %real_fbrfiles, %wrongexec_files, %binfiles,
     %txtfiles, %files, %svn_files, %alias, %modules,
     @params, @versions,
     $key, $name, $type, $first, $arg, $verbose, $readlink, $fbrbase);

$fbrbase = "../bin/$ARCH";

# --- SUBS ---
sub process_file {
    my ($pkg, $file, $arr) = @_;
    foreach my $pat (@ignore) {
        return if $file =~ /$pat/;
    }
    $$arr{$pkg}{$file} = 1;
}

sub process_any_file {
    my ($file) = @_;
    my $fname = $file; $fname =~ s#packages/[^/]+/##;
    my $pkg = $file; $pkg =~ s#packages/([^/]+)/.*#$1#;
    process_file($pkg, $fname, \%real_files);
    if ($file =~ /^packages\/[^\/]+\/opt\/.+/ && -x $file) {
        $wrongexec_files{$file} = 1;
    }
}

sub process_nonlink_file {
    my ($file) = @_;
    process_file("", $file, \%real_nonlink_files);
}

sub process_fbr_file {
    my ($file) = @_;
    $file =~ s/^\.\.\/bin\/$ARCH\///;
    if ($file =~ /^packages\//) {
        my $fname = $file; $fname =~ s#packages/[^/]+/##;
        my $pkg = $file; $pkg =~ s#packages/([^/]+)/.*#$1#;
        if ($fname =~ /^(check|config|doc|img|opt)\//) {
            process_file($pkg, "$fname", \%real_fbrfiles);
        }
        else {
            process_file($pkg, "opt/$fname", \%real_fbrfiles);
        }
    }
    else {
        process_file("", "opt/$file", \%real_fbrfiles);
    }
}

# return all keys from the first hash that are not present in the second hash
sub weedout {
    my ($one, $two) = @_;
    grep { !defined($two->{$_}) } keys(%$one);
}
sub weedout3 {
    my ($one, $two, $three, $four) = @_;
    grep { !defined($two->{$_}) && !defined($three->{$_}) && !defined($four->{$_}) } keys(%$one);
}

sub print_verb {
    my ($txt, $serverity) = @_;
    return unless $verbose || $serverity;

    if ($first) {
        print "$name:\n";
        $first = 0;
    }
    print "    $txt";
}
# --- MAIN ---

if (@ARGV == 0) {
    @params = ('files', 'opts', 'svn', 'props', 'libs');
} else {
    @params = @ARGV;
}

print "* * * * * * * * * *\n";
print "* check-files.pl  *\n";
print "* * * * * * * * * *\n";

#
# determine readlink
#
$readlink = `which grealpath 2>/dev/null`;
chomp($readlink);
if (!$readlink) {
	$readlink = `which realpath 2>/dev/null`;
	chomp($readlink);
}
if (!$readlink) {
	$readlink = `which readlink 2>/dev/null`;
	chomp($readlink);
	if ($readlink) {
		$readlink .= ' -f';
	}
}
if (!$readlink) {
	print "Error: Neither (g)realpath nor readlink has been found on your host!\n";
	exit(2);
}

print "\nSearching for files... create filelists...";
print "\n------------------------------------------\n";

#
# get list of directories to search
#
my @dirs = ();
foreach my $dir (sort <packages/*>) {
    # ignore files and internal directories
    push (@dirs, $dir) unless ! -d "$dir" || $dir =~ /^_.*/;
}

#
# read files in directories
#
my @fs_files = `find -L @dirs -not -type d`;
foreach my $fs_file (@fs_files) {
    chomp($fs_file);
    process_any_file($fs_file);
}

my @fs_nonlink_files = `find @dirs -not -type d`;
foreach my $fs_nonlink_file (@fs_nonlink_files) {
    chomp($fs_nonlink_file);
    process_nonlink_file($fs_nonlink_file,);
}

my @fs_fbrfiles = `find "$fbrbase/" -not -type d`;
foreach my $fs_fbrfile (@fs_fbrfiles) {
    chomp($fs_fbrfile);
    process_fbr_file($fs_fbrfile);
}

my @fs_kernels = `find "$fbrbase/img" -type d -name 'linux-*'`;
foreach my $fs_kernel (@fs_kernels) {
    chomp($fs_kernel);
    $fs_kernel =~ s#^.*/linux-([^/]*)$#$1#;
    push(@versions, $fs_kernel);
}

#
# read files.txt
#
sub process_files_txt {
    my ($pkg, $dir) = @_;
    my $p = $pkg; $p =~ s#^packages/##;
    my $list = "$dir/$pkg/$filelist";
    open (FILES, "<", "$list") or return;

    while (<FILES>) {
        /^(.) (([^:]+)::)?(.*)$/o;
        $type = $1;
        my $arch = defined($3) ? $3 : $ARCH;
        next if $arch ne $ARCH;
        my $fname = $4;

        $name = "$dir/$pkg/$fname";
        $binfiles{$p}{$fname} = 1 if ( $type =~ /[bB]/);
        $txtfiles{$p}{$fname} = 1 if ( $type =~ /[dDuU]/);
        $files{$p}{$fname} = 1;
    }
    close FILES;
}

foreach my $pkg (@dirs) {
    process_files_txt($pkg, ".");
    process_files_txt($pkg, $fbrbase);
}

foreach $arg (@params) {
    if ($arg eq "files") {
        print "Missing files:";
        print "\n--------------\n";
        foreach my $pkg (@dirs) {
            my $p = $pkg; $p =~ s#^packages/##;
            my @missing = sort(weedout3($files{$p},$real_files{$p},$real_fbrfiles{$p},$real_fbrfiles{""}));
            print "! $p ".join("\n! $p ", @missing)."\n" unless $#missing < 0;
        }

        print "\nNot in files.txt:";
        print "\n-----------------\n";
        foreach my $pkg (@dirs) {
            my $p = $pkg; $p =~ s#^packages/##;
            my @missing = sort(weedout($real_files{$p},$files{$p}));
            print "? $p ".join("\n? $p ", @missing)."\n" unless $#missing < 0;
        }
    } elsif ($arg eq "opts") {
        print "\nChecking opt/*.txt files ...";
        print "\n----------------------------\n";
        my %opt_files = ();
        my %stripped_files = ();

        # create a hash without leading packet names and opt prefix
        foreach my $pkg (@dirs) {
            my $p = $pkg; $p =~ s#^packages/##;
            if (defined($files{$p})) {
                foreach (keys %{$files{$p}}) {
                    $stripped_files{$_} = 1;
                }
            }
            if (defined($real_fbrfiles{$p})) {
                foreach (keys %{$real_fbrfiles{$p}}) {
                    $stripped_files{$_} = 1;
                }
            }
        }

        # filter out all kernel modules
        foreach (keys %files) {
            foreach (keys %{$files{$_}}) {
                if (m#^opt/lib/modules/([^/]+)/.*/([^/]+).ko#) {
                    my $version = $1;
                    my $name = $2; $name =~ s#_#-#g;
                    $modules{$version}{$name} = 1;
                }
            }
        }

        # add aliases
        for my $version (@versions) {
            my $name = $fbrbase . '/lib/modules/' . $version . '/modules.alias';
            open (ALIAS, "<", $name) || die "Unable to open '$name'";
            while (<ALIAS>) {
                next if /^#.*|.*:.*|^[[:space:]]*$/;
                my ($foo, $alias, $bar) = split;
                $modules{$version}{$alias} = 1;
            }
        }

        foreach my $dir (@dirs) {
            my $pkg = $dir; $pkg =~ s#^packages/##;
            next unless -f "$dir/opt/$pkg.txt";
            for my $file (sort glob("$dir/opt/*.txt")) {
                my $OPT;
                $name = $file;
                $first = 1;
                open (OPT, "<", $file);
                while (<OPT>) {
                    # skip comments, format indicator, weak declaration
                    next if (/^(#|opt_format|weak).*/);
                    # skip empty lines
                    next if (/^\s*$/);
                    # skip non-files
                    next if (/^\s*\S+\s+\S+\s+\S+\s+type=dir\b.*$/);
                    next if (/^\s*\S+\s+\S+\s+\S+\s+type=node\b.*$/);
                    next if (/^\s*\S+\s+\S+\s+\S+\s+type=symlink\b.*$/);
                    # remove rootfs prefixes
                    s/rootfs://;
                    # remove trailing comments
                    s/\#.*//;

                    my ($var, $val, $name) = split;

                    if ($name =~ /.*\/.*/) {
                        if ($name =~ /\$\{[^}]+\}/) {
                            # print "checking $name\n";
                            $name =~ s#\$\{[^}]+\}#[^/]*#;
                            my @res = grep {$_ =~ m#opt/$name#} keys %stripped_files;
                            if ($#res < 0) {
                                @res = grep {$_ =~ m#opt/$name#} keys %{$real_fbrfiles{""}};
                            }
                            if ($#res < 0) {
                                print_verb("$name\n", 1);
                            }
                        } else {
                            next if defined $files{$pkg}{"opt/$name"};
                            next if defined $real_fbrfiles{$pkg}{"opt/$name"};
                            next if defined $real_fbrfiles{""}{"opt/$name"};
                            next if $name eq "etc/rc.cfg";
                            # print "checking $name\n";
                            my @res = grep {$_ =~ m#opt/$name#} keys %stripped_files;
                            if ($#res < 0) {
                                @res = grep {$_ =~ m#opt/$name#} keys %{$real_fbrfiles{""}};
                            }
                            if ($#res < 0) {
                                print_verb("$name\n", 1);
                            }
                        }
                        next;
                    }
                    # check presence of kernel module in all kernel versions
                    if ($name =~ /^(.*)\.ko$/) {
                        my $modname = $1; $modname =~ s#_#-#g;
                        my %found = ();
                        for my $version (@versions) {
                            if (defined $modules{$version}{$modname}) {
                                $found{$version} = 1;
                            }
                        }
                        if (scalar keys(%found) != scalar keys(%modules)) {
                            my $versions = "";
                            foreach my $v (sort(weedout(\%modules, \%found))) {
                                $versions = "$versions $v";
                            }
                            print_verb("$name (missing in$versions)\n", 1);
                        }
                    }
                }
            }
        }
    } elsif ($arg eq "svn") {
        my $file;

        print "\nFiles not in repository:";
        print "\n------------------------\n";

        open (SVN, "svn info --depth=infinity packages |") || die "unable to fork svn";
        while (<SVN>) {
            chomp;
            if(/^Path:\s+/) {
                $file = $_;
                $file =~ s/^Path:\s+//;
                next if $file eq ".";
            }
            if(/^Schedule:\s+(normal|add|delete|replace)$/) {
                $svn_files{$file} = 1 ;
            }
        }
        close SVN;

        my @missing = sort(weedout($real_nonlink_files{""},\%svn_files));
        print "R ".join("\nR ", @missing)."\n" unless $#missing < 0;
    } elsif ($arg eq "props") {
        print "\nChecking file svn properties...";
        print "\n-------------------------------\n";
        my $ignore_txt = 0;
        my $ignore_bin = 0;
        my $pkg = "";
        my $fname = "";
        open (SVN, "svn proplist -v --xml -R packages |") || die "unable to fork svn";
        while (<SVN>) {
            chomp;
            if (/path=\"(.*)\"/) {
                $name=$1;
                $fname=$name; $fname =~ s#packages/[^/]+/##;
                $pkg=$name; $pkg =~ s#packages/([^/]+)/.*#$1#;

                $ignore_txt = !defined($txtfiles{$pkg}{$fname});
                $ignore_bin = !defined($binfiles{$pkg}{$fname});

                next;
            }
            if (!$ignore_txt && / *name=\"svn:keywords\"/) {
                $type = $_;
                $type =~ s/ *name=\"svn:keywords\">//;
                $type =~ s/<\/property>//;
                next if ($type =~ /Author\s+Date\s+Id\s+Revision$/);
                print "T $name svn:keywords $type\n";
            }
            elsif (!$ignore_bin) {
                next if ! / *name=\"/;
                next if /name=\"svn:mime-type\">application\/octet-stream<\/property>/;
                next if /name=\"svn:mime-type\">application\/x-sharedlib<\/property>/;
                next if /name=\"svn:special\"/;
                if ($name !~ /^[^\/]+\/opt\/.+/) {
                    next if /svn:executable/;
                    next if /svn:mime-type/;
                }
                my $property = $_;
                $property =~ s/ *name=\"//;
                $property =~ s/\">/ /;
                $property =~ s/<\/property>//;
                print "B $name $property\n";
            }
        }
        close SVN;

        print "\nFiles wrongly marked as executable:";
        print "\n-----------------------------------\n";
        my @wrongexec = sort(keys %wrongexec_files);
        if (@wrongexec > 0) {
            print "E ".join("\nE ", @wrongexec)."\n";
        }
    } elsif ($arg eq "libs" || $arg eq "libdeps") {
        print "\nChecking library dependencies...";
        print "\n--------------------------------\n";
        my $ldcache=".ldcache-$ARCH";
        system("./resolve-libs.sh --create-ld-cache $ldcache ../bin/$ARCH");
        my $pkg = "";
        my $nl_needed = 0;

        my @result = ();
        foreach my $dir (@dirs) {
            my $pkg = $dir; $pkg =~ s#^packages/##;
            copy($ldcache, "$ldcache.$pkg");

            foreach my $prog (sort (keys %{$files{$pkg}})) {
                next unless $prog =~ /^opt\//;
                next unless defined($binfiles{$pkg}{$prog});
                next if $prog =~ /.*\.ko$/;
                next if $prog =~ /(^|.*\/)firmware\/.*/;
                next if $prog =~ /^img\/.*/;
                next if $prog =~ /\.(png|gif|jpg|pdf|gz|tgz|bz2|lzma|bin|ttf|pif|com|exe|ico|chm|cgi|mo|msg|map)$/;

                if (-e "$dir/$prog") {
                    system("./resolve-libs.sh", "--fill-rpath-cache", "$ldcache.$pkg", "$dir/$prog", "../bin/$ARCH", "base" ,"$pkg");
                }
                else {
                    $prog =~ s#^opt/##;
                    system("./resolve-libs.sh", "--fill-rpath-cache", "$ldcache.$pkg", "$fbrbase/$prog", "../bin/$ARCH", "base", "$pkg");
                }
            }
            system("sort -u -o $ldcache.$pkg $ldcache.$pkg");

            foreach my $prog (sort (keys %{$files{$pkg}})) {
                next unless $prog =~ /^opt\//;
                next unless defined($binfiles{$pkg}{$prog});
                next if $prog =~ /.*\.ko$/;
                next if $prog =~ /(^|.*\/)firmware\/.*/;
                next if $prog =~ /^img\/.*/;
                next if $prog =~ /\.(png|gif|jpg|pdf|gz|tgz|bz2|lzma|bin|ttf|pif|com|exe|ico|chm|cgi|mo|msg|map)$/;

                my $line = "$pkg $prog";
                my $len = length($line);
                if ($len >= LINELEN) {
                    $line = substr($line, 0, LINELEN - 4);
                    $line .= "...";
                }
                print("\033[2K\r$line");
                $nl_needed = 1;

                if (-e "$dir/$prog") {
                    @result = `./resolve-libs.sh --recursive --use-ld-cache $ldcache.$pkg -- "$dir/$prog" ../bin/$ARCH base $pkg`;
                }
                else {
                    $prog =~ s#^opt/##;
                    @result = `./resolve-libs.sh --recursive --use-ld-cache $ldcache.$pkg -- "$fbrbase/$prog" ../bin/$ARCH base $pkg`;
                }

                my @deps = grep(/^-.*/, @result);
                foreach (@deps) {
                    chomp;
                    s/^-//;
                }
                if ($#deps >= 0) {
                    $nl_needed = 0;
                    print "\n    ".join("\n    ", @deps)."\n";
                }
            }

            unlink("$ldcache.$pkg");
        }
        if ($nl_needed) {
            print("\033[2K\n");
        }
        system("./resolve-libs.sh --destroy-ld-cache $ldcache");
    } else {
        print <<USAGE

Usage: check-files.pl [ files | opts | svn | props | libs | libdeps ]\n
    default: files opts svn props libs

    files - check whether all files referenced in files.txt are present and
            whether all present files are referenced in files.txt

    opts  - checks whether all files referenced in opt/*.txt are present
            in files.txt

    svn  - checks wheter all files are under version control

    props - checks whether all files have the corrent svn properties

    libs - checks whether all needed libraries are present in files.txt
           (uses readelf to get dependencies)

    libdeps - creates a list of library dependencies
USAGE
    }
}
