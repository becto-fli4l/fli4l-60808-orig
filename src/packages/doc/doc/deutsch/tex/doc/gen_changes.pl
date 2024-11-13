#! /usr/bin/perl -w

# language constants
$_GC_differences = "Unterschiede Version";
$_GC_and = "und";
$_GC_package = "Package";
$_GC_renamed_vars = "Umbenannte Variablen";
$_GC_obsolete_vars = "Gelöschte Variablen";
$_GC_new_vars = "Neue Variablen";
$_GC_original_name = "Originaler Name";
$_GC_new_name = "Neuer Name";
$_GC_new = "neu";
$_GC_obsolete = "gelöscht";
$_GC_renamed = "umbenannt";

use FindBin;

$program_dir=$FindBin::Bin;

require "$program_dir/../../../common/tex/doc/gen_changes.pm";
