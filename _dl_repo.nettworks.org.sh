#!/bin/sh

# \\_____
# // README:
# - download the original sources and binary files from fli4l repository

echo "-> remove src from working copy" && rm -Rf ./src
echo "-> get src-directory from original repo" \
	&& git svn clone -r60808 -T branches/4.0/trunk/src https://repo.nettworks.org/svn/fli4l ./src

echo "-> remove ./bin/x86_64 from working copy" && rm -Rf ./bin/x86_64
echo "-> get bin/x86_64 from original repo" \
	&& cd ./bin \
	&& svn checkout -r60808 https://repo.nettworks.org/svn/fli4l/branches/4.0/trunk/bin/x86_64

# ---//
