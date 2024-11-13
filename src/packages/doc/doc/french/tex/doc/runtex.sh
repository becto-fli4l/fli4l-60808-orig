#! /bin/bash
# set -x
dump_error ()
{
    while read line
    do
        case "$line" in
            *tex:*:*)
                echo
                echo "Error: $line"
                set $line
                # set -x 
                set `echo $1 | sed -e 's/:/ /g'`
                text="`sed -n -e $2p $1`"
                echo "Text: '$text'"
                if echo "$text" | grep -e '[^\]_' -e '^_'
                then
                    echo "Please use '\\_' instead of '_'"
                fi
                if echo "$text" | grep -e '"'
                then
                    echo "Please use ``foo'' or \flqq{}foo\frqq{} instead of \"foo\""
                fi
                
                set +x
                ;;
            *)
                echo "Comment: $line"
                ;;
        esac
    done
}

if [ $# -ne 3 ]
then
    echo usage "$0 <latex|pdflatex> <file name> <interactive|errorstopmode>"
    exit 1
fi

package=`echo $2 | sed -e 's/\.tex//'`

rm -f tex_errors
$1  -file-line-error-style -interaction=$3 $2

if [ $? -ne 0 ]
then
    grep -e "^.*\.tex:[0-9]\+:" $package.log > tex_errors
cat <<EOF
#
# There are errors in your tex document
#
EOF
    sed -n -e '/.*.tex:[0-9]\+:.*/,/^[[:space:]]*$/ { p }' $package.log | dump_error
cat <<EOF
#
# Error Summary
#
EOF
    cat tex_errors | dump_error
    if [ "$1" = "latex" ]
    then
        rm -f $package.dvi $package.ps
    elif [ "$1" = "pdflatex" ]
    then
        rm -f $package.pdf
    fi
    exit 1
fi

egrep -q -e '^LaTeX Warning: There were undefined references\.$' \
    -e '^LaTeX Warning: Label\(s\) may have changed\. Rerun to get cross-references right\.$' \
    $package.log >/dev/null 2>&1

if [ $? -eq 0 ]
then
    $1  -interaction=$3 $2
fi

makeindex $package
sed -i -e 's/\\_/\\_\\-/g;s/\\discretionary {-}{}{}//g' $package.ind
cp $package.idx $package-idx.bak
$1 -interaction=$3 $2
$1 -interaction=$3 $2
diff -q $package-idx.bak $package.idx
if [ $? -ne 0 ]
then
    makeindex $package
    sed -i -e 's/\\_/\\_\\-/g;s/\\discretionary {-}{}{}//g' $package.ind
    $1 -interaction=$3 $2
fi

egrep -q -e '^LaTeX Warning: There were undefined references\.$' \
    -e '^LaTeX Warning: Label\(s\) may have changed\. Rerun to get cross-references right\.$' \
    $package.log >/dev/null 2>&1

if [ $? -eq 0 -o "$1" = "pdflatex" ]
then
    $1 -interaction=$3 $2
fi

rm -f $package-idx.bak
