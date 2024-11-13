#! /usr/bin/perl -w

# language constants
$_GC_differences = "Differences version";
$_GC_and = "and version";
$_GC_package = "Package";
$_GC_renamed_vars = "Renamed variables";
$_GC_obsolete_vars = "Obsolete variables";
$_GC_new_vars = "New variables";
$_GC_original_name = "Original name";
$_GC_new_name = "New name";
$_GC_new = "new";
$_GC_obsolete = "obsolete";
$_GC_renamed = "renamed";

use FindBin;

$program_dir=$FindBin::Bin;

require "$program_dir/../../../common/tex/doc/gen_changes.pm";
