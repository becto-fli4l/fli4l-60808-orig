#! /bin/bash

if [ $# != 1 ]
then
    echo "usage: conv.sh <package name>"
    exit 1
fi

if [ ! -f $1.txt ]
then
    echo "$1.txt not found, aborting"
    exit 1
fi

if [ -f Makefile -o -f $1.tex -o -f $1_main.tex ]
then
    echo "Makefile, $1.tex or $1_main.tex already exist, aborting"
    exit 1
fi

# create Makefile
cat > Makefile <<EOF
ifeq (\$(DOCDIR),)
	DOCDIR := \$(shell sh -c "if [ -d ../../../../../doc/doc/deutsch/tex/doc ]; then echo '../../../../../doc/doc/deutsch/tex/doc'; elif [ -d ../doc ]; then echo '../doc'; fi")
	ifeq (\$(DOCDIR),)
		DOCDIRS := \$(error "Please install the fli4l documentation package, which contains all necessary files to build ps/pdf/html documentation")
	endif
endif
include \$(DOCDIR)/Make.conf
EOF

PACKAGE=`echo $1 | tr "[:lower:]" "[:upper:]"`

# create main tex file
cat > $1.tex <<EOF

\include{fli4l}
\makeindex

\begin{document}

\newcommand{\titlename}{$CONV_package $PACKAGE}

\flhypersetup{pdftitle=\titlename}
\pdfbkmrk{-1}{\titlename}{title}
\title{\titlename\\\\ $CONV_version \version}

\author{$CONV_team\\\\ \email{team@fli4l.de}}

\maketitle
\pdfbkmrk{0}{\contentsname}{table}
\tableofcontents
\chapter{$CONV_doc $PACKAGE}

\input {$1_main}

\appendix
\chapter{$CONV_appendix $PACKAGE}

\input{$1_appendix}
\listoffigures
\listoftables
\printindex

\end{document}

EOF
# convert txt file
cat $1.txt | $dir/../../../common/tex/doc/conv.pl > $1_main.tmp
iconv -f iso-8859-1 -t utf-8 $1_main.tmp -o $1_main.tex
rm $1_main.tmp

if [ -f $dir/../../../../../base/version.txt ]
then
    # SVN
    destdir=$dir/../../../../../$1/doc/deutsch/tex/$1
else
    # Distro
    destdir=$dir/../$1
fi

mkdir -p $destdir
for file in Makefile $1.tex $1_main.tex
do
    if [ -f $destdir/$file ]
    then
        echo "Makefile, $1.tex or $1_main.tex already exist in desired destination directory:"
        echo "$destdir"
        echo "LaTeX files for package $1 stored in:"
        echo `pwd` 
        exit 1
    fi
done
for file in Makefile $1.tex $1_main.tex
do
    mv $file $destdir
done
echo "LaTeX files for package $1 stored in:"
echo "$destdir"
