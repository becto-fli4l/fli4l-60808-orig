#! /bin/sh

for package in packages/*
do
    if [ -d $package ]
    then
        for lang in $package/doc/*
        do
            for doc in $lang/tex/*
            do
                if [ -f $doc/Makefile ]
                then
                    echo "cleaning $doc"
                    make -C $doc distclean > /dev/null
                fi
                for app in $doc/*_appendix.tex
                do
                    if [ -f $app ]
                    then
                        if [ ! -s $app ]
                        then
                            echo "deleting empty appendix $app"
                            rm -f $app
                        fi
                    fi
                done
            done
        done
    fi
done

echo "cleaning packages/base/src"
make -C packages/base/src clean > /dev/null
