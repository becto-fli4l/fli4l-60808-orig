#!/bin/bash
# depends on: readlink, mktemp, sed, sort, svn, flatten, latexdiff, pdflatex
#             and everything we need to build the fli4l PDF documentation

if [ $# -ne 2 -a $# -ne 4 ]
then
	echo "usage: $(basename $0) <repo-url> <output-directory> [<rev-old> <rev-new>]" >&2
	exit 1
fi

OUTPUT="$(readlink -f "$2")"
if [ ! -d "$OUTPUT" ]
then
	echo "error: could not find output directory $OUTPUT" >&2
	exit 2
fi

ROOT=$(mktemp -d)
if [ ! -d "$ROOT" ]
then
	echo "error: could not create temporary directory" >&2
	exit 3
else
	trap "rm -rf \"$ROOT\"" 0
fi

if [ -d "$1" ]
then
	URL="$(readlink -f "$1")"
else
	URL="$1"
fi
URL="$URL/src/packages"

if [ $# -eq 2 ]
then
	SAFE_URL="$(echo "$URL" | sed 's/\//\\\//g')"
	SVN_CMD="svn status \"$URL\" | sed -n 's/^M *$SAFE_URL\\/\\([^/]\\+\\)\\/doc\\/\\([^/]\\+\\)\\/.*/\\1 \\2/p'"
else
	SVN_CMD="svn log -v -r $(($3+1)):$4 \"$URL\" | sed -n 's/^   M \\/\\(trunk\\|branches\\/[^ ]\\+\\/[^ ]\\+\\)\\/src\\/packages\\/\\([^/]\\+\\)\\/doc\\/\\([^/]\\+\\)\\/.*/\\2 \\3/p'"
fi

eval "$SVN_CMD" | sort -u | sed '/^doc /d' | while read package lang
do
	echo "retrieving sources for $package ($lang)..." >&2
	rm -rf $ROOT/old $ROOT/new
	for p in $package doc
	do
		if [ "$p" = "base" ]
		then
			mkdir -p $ROOT/old $ROOT/new
			P=$p
		else
			mkdir -p $ROOT/old/$p $ROOT/new/$p
			P=$p/doc
		fi
		if [ $# -eq 2 ]
		then
			svn -q export -r HEAD $URL/$P $ROOT/old/$P
			svn -q export $URL/$P $ROOT/new/$P
		else
			svn -q export -r $3 $URL/$P $ROOT/old/$P
			svn -q export -r $4 $URL/$P $ROOT/new/$P
		fi
	done

	echo "building documentation..." >&2
	pushd $ROOT/old/$package/doc/$lang/tex/$package >/dev/null &&
			make pdflink >/dev/null 2>&1 </dev/null &&
			FLATINPUTS=$ROOT/old/$package/doc/common/tex/$package flatten $package.tex $ROOT/${package}_old.tex >/dev/null || {
		echo "error: building old documentation failed, skipping" >&2
		popd >/dev/null
		continue
	}
	popd >/dev/null
	pushd $ROOT/new/$package/doc/$lang/tex/$package >/dev/null &&
			make pdflink >/dev/null 2>&1 </dev/null &&
			FLATINPUTS=$ROOT/new/$package/doc/common/tex/$package flatten $package.tex $ROOT/${package}_new.tex >/dev/null || {
		echo "error: building new documentation failed, skipping" >&2
		popd >/dev/null
		continue
	}
	popd >/dev/null

	echo "determining changes..." >&2
	if [ $# -eq 2 ]
	then
		name=$package
	else
		name=$3-$4-$package
	fi
	latexdiff -t CULINECHBAR $ROOT/${package}_old.tex $ROOT/${package}_new.tex > $ROOT/new/$package/doc/$lang/tex/$package/$package.tex 2>/dev/null || {
		echo "error: determining changes failed, skipping" >&2
		continue
	}
	sed -i 's/\\PERCENTAGE\>/\\%/g' $ROOT/new/$package/doc/$lang/tex/$package/$package.tex

	echo "building resulting PDF file $name..." >&2
	pushd $ROOT/new/$package/doc/$lang/tex/$package >/dev/null
	for type in lan wlan
	do
		for arch in x86 x86_64
		do
			> niclist_${type}-${arch}.csv
		done
	done
	make $package.pdf >/tmp/latexdiff.log 2>&1 </dev/null
	[ -f $package.pdf ] && {
		mkdir -p $OUTPUT/$lang
		mv $package.pdf $OUTPUT/$lang/$name.pdf
		echo $lang/$name.pdf
	} ||
		echo "error: could not create $name.pdf, skipping" >&2
	popd >/dev/null
done
exit 0
