#! /usr/bin/perl -w

# language constants
$_GC_differences = "Différences entre la version";
$_GC_and = "et";
$_GC_package = "Paquetage";
$_GC_renamed_vars = "Variables retitrées";
$_GC_obsolete_vars = "Variables supprimées";
$_GC_new_vars = "Nouvelles variables";
$_GC_original_name = "Nom original";
$_GC_new_name = "Nouveau nom";
$_GC_new = "nouveau";
$_GC_obsolete = "supprimé";
$_GC_renamed = "retitré";


use FindBin;

$program_dir=$FindBin::Bin;

require "$program_dir/../../../common/tex/doc/gen_changes.pm";
