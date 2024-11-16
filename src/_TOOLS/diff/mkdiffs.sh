def_ignore='windows/autoit/*'

gen_single_diff ()
{
    for i in $1/*; do
	if svn info $i | grep -q ^Path ; then
	    if ! _TOOLS/diff/svndiff $version $i 2> /tmp/gendiff.$$; then
		if ! grep -q ".*tags.*not.*found" /tmp/gendiff.$$; then
		    # last try
		    echo -n _
		    if ! _TOOLS/diff/svndiff -n $version $i 2> /tmp/gendiff.$$; then
			cat /tmp/gendiff.$$
			return 1;
		    fi
		fi
	    fi
	fi
    done
}

gen_diff ()
{
    if ! _TOOLS/diff/svndiff $version $1 >> $2 2> $3; then
	if grep -q ".*tags.*not.*found" $3; then
	    return 1;
	fi
	echo -n "retrying ... "
	if ! gen_single_diff $1 >> $2 2>> $3; then
	    return 1
	fi
    fi
}

gen_html ()
{
    cat <<EOF
<html>
<head>
    <title>Diff for `basename $1 .diff`</title>
</head>
<body>

<h1>Summary</h1>
    <table border=1>
    <!--
	<colgroup>
	    <col width="200">
	    <col width="30">
	    <col width="100">
	</colgroup>
     -->
EOF
    
    diffstat -k -p 0 $1 | _TOOLS/diff/filter_diff.pl stat -x "$ignore"
    
    cat <<EOF
    </table>
<h1>Changed Binaries</h1>
<pre>
EOF
    
    if ! _TOOLS/diff/filter_diff.pl bin $1; then
	echo "None"
    fi
    
    cat <<EOF
</pre>

<h1>Changes</h1>
EOF
    
    if ! _TOOLS/diff/filter_diff.pl changes $1; then
	echo "None"
    fi
    
    cat <<EOF
</pre>
<h1>Diff</h1>
EOF

    perl _TOOLS/diff/filter_diff.pl filter -x $ignore $1

    cat <<EOF
</body>
</html>
EOF

}

do_diff ()
{
    printf "%-25s ... diff ... " $pkg 
    if ! gen_diff $pkg $target_dir/$pkg.diff $target_dir/$pkg.err; then
	echo 
	echo -n "    "
	cat $target_dir/$pkg.err
	rm -f $target_dir/$pkg*
	return;
    fi
    rm -f $target_dir/$pkg.err

    found=
    file=$target_dir/$pkg.diff
    if [ -s $file ]; then
	for name in `sed -n -e 's/^Index: //p' $file`; do
	    case $name in
		*changes/*) ;;
		*) found=yes ;;
	    esac
	done
    fi
    if [ "$found" ]; then
	echo -n "html ... "
	gen_html $file > $target_dir/$pkg.html
    else
	echo -n "no changes ... "
	rm -f $file
    fi
    echo " finished"
}

do_stat ()
{
    cat <<EOF
    <table border=0>
    <!--
	<colgroup>
	    <col width="50">
	    <col width="200">
	    <col width="20">
	    <col width="100">
	</colgroup>
     -->
	<tr>
	    <th>package</th>
	    <th>file</th>
	    <th>lines</th>
	    <th>statistics</td>
	</tr>

EOF
    while [ "$1" ]; do
	pkg=$1
	diff=$target_dir/$pkg.diff
	if [ -f $diff ]; then
	    cat <<EOF
        <tr>
	    <td colspan=4><A HREF="$pkg.html">$pkg</A> (<A HREF="$pkg.diff">plain diff</A>)
        <tr>
EOF
            diffstat -k -p 0 $diff | _TOOLS/diff/filter_diff.pl stat -x "$ignore" $pkg.html 
	fi
	shift
    done
    
    echo "    </table>"
}

usage ()
{
    [ -d _RELEASE ] || echo "unable to locate _RELEASE directory"
    echo "usage: $0 <version> <target_dir> [ -i <exclude pattern>] [ packet list ]"
    exit 1
}

[ $# -lt 2 ] && usage
[ -d _RELEASE ] || usage

version=$1
target_dir=$2
shift 2

ignore="$def_ignore"
case $1 in
    -i*) 
        [ "$2" ] || usage
        ignore="$def_ignore,$2"
        shift 2
        ;;
esac

if [ "$1" ]; then
    pkgs="$*"
else
    . _RELEASE/release.conf
    pkgs=$packages
    . _RELEASE/release-development.conf
    pkgs="$pkgs $packages"
    . _RELEASE/release-non_gpl.conf
    pkgs="$pkgs $packages"
    . _RELEASE/release-opt_db.conf
    pkgs="$pkgs $packages"
fi

if [ ! -d $target_dir ]; then
    mkdir -p $target_dir
else
    rm -f $target_dir/*.html
    rm -f $target_dir/*.diff
fi

    
for pkg in  $pkgs; do
    case $pkg in
	avm*) 
	    ;;
        src)
            # the src-package has non opt subdir
            file=$target_dir/$pkg.diff
            do_diff $pkg
            ;;
	*)
            [ -d $pkg/opt ] || continue
	    file=$target_dir/$pkg.diff
	    do_diff $pkg
	    ;;
    esac
done

{
    cat <<EOF
<html>
<head>
    <title>Diff against Version $version</title>
</head>
<body>
    <H1>Diff against Version $version</H1>
    <H2>Main packages - Diff against Version $version</H2>
EOF
    . _RELEASE/release.conf
    all="$packages"
    do_stat $packages

    echo "<H2>Non-GPL packages - Diff against Version $version</H2>"
    . _RELEASE/release-non_gpl.conf
    all="$all $packages"
    do_stat $packages

    echo "<H2>OPT-DB packages - Diff against Version $version</<H2>"
    . _RELEASE/release-opt_db.conf
    all="$all $packages"
    do_stat $packages

#    set -x
    for i in */opt/*.txt; do
	pkg=`echo $i | sed -e 's#/.*##'`
	if ! echo "$all" | grep -q -w $pkg; then
	    missing="$missing $pkg"
	fi
    done

    echo "<H2>Other packages - Diff against Version $version</H2>"
    do_stat $missing
    cat <<EOF
</body>
</html>
EOF
} > $target_dir/index.html
