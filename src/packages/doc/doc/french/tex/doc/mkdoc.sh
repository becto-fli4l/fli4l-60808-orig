#!/bin/bash

if [ "$ROOT_DIR" = "" ]
then
    root_dir=../../../../..
else
    root_dir=$ROOT_DIR
fi

lang=french
tex_dir=doc/$lang/tex
base_dir=$root_dir/base
base_doc_dir=$root_dir/base/$tex_dir/base
dev_doc_dir=$root_dir/base/$tex_dir/dev

. $(dirname $(readlink -f $0))/../../../common/tex/doc/common_doc.conf.$ARCH

link_pics ()
{
    for j in $1/*.fig $1/*.png $1/*.eps
    do
        if [ -f $j ]
        then
            ln -sf $j .
        fi
    done
}

create_empty_doc ()
{
    heading=`echo $1 | sed -e "s/_/\\\\\\_/g" | tr "[:lower:]" "[:upper:]"`
    rm -f $2
    cat > $2 <<EOF

    \\section {Paquetage $heading}

    Actuellement la documentation basée en \\LaTeX \\ pour ce
    paquetage n'est pas encore disponible.

EOF
}

create_error_doc ()
{
    heading=`echo $1 | sed -e "s/_/\\\\\\_/g" | tr "[:lower:]" "[:upper:]"`
    rm -f $2
    cat > $2 <<EOF

    \\section {Erreurs de tex pour paquetage $heading}

    Pour ce paquetage une documentation \\LaTeX \\ existe, mais
    il n'est pas possible de la traduire. Les erreurs sont les suivantes~:
....
    \begin{verbatim}
EOF
    cat $3 >> $2
    cat >> $2 <<EOF
    \end{verbatim}
EOF
}
create_links ()
{
    printf "%-30s" "Package base:"
    if [ -f $base_doc_dir/tex_errors  ]
    then
        echo "errors in tex file"
        create_error_doc base base_main.tex $base_doc_dir/tex_errors
        touch base_middle.tex
        touch base_winimonc.tex
        touch base_appendix.tex
    else
        echo "main      middle, winimonc, appendix"
        for f in $base_doc_dir/base_*.tex; do
            ln -sf $f .
        done
    fi

    printf "%-30s" "Package dev:"
    if [ -f  $dev_doc_dir/tex_errors ]
    then
        echo "errors in tex file"
        create_error_doc Developer dev_main.tex $dev_doc_dir/tex_errors
        touch dev_appendix.tex
    elif [ -f $dev_doc_dir/dev_appendix.tex ]
    then
        echo "main      appendix"
        for f in $dev_doc_dir/dev_*.tex; do
           ln -sf $f .
        done
        #ln -sf $dev_doc_dir/dev_main.tex .
        #ln -sf $dev_doc_dir/dev_appendix.tex .
    else
        echo "main      no appendix"
        for f in $dev_doc_dir/dev_main*.tex; do
           ln -sf $f .
        done
        touch dev_appendix.tex
    fi
    link_pics $base_doc_dir
    link_pics $dev_doc_dir

    for name in $opt_list
    do
        dir=$root_dir/$name/$tex_dir/$name
        file=$name'_main.tex'
        printf "%-30s" "Package $name:"
        if [ -f $dir/tex_errors ]
        then
            echo "errors in tex file"
            create_error_doc $name $file $dir/tex_errors
        elif [ -f $dir/$file ]
        then
            echo -n "main      "
            ln -sf $dir/$file .
            link_pics $dir
        else
            echo -n "no main   "
            create_empty_doc $name $file
        fi

        file=$name'_appendix.tex'
        if [ -f $dir/tex_errors ]
        then
            rm -f $file
            touch $file
        elif [ -f $dir/$file ]
        then
            echo "appendix"
            ln -sf  $dir/$file .
        else
            rm -f $file
            echo "no appendix"
            touch $file
        fi
    done
}

remove_links ()
{
    for i in base dev $opt_list
    do
        main=$i'_main.tex'
        appendix=$i'_appendix.tex'
        rm -f $main $appendix
    done
    rm -f base_*.tex dev_*.tex

    for j in *.fig *.png *.eps
    do
        if [ -L $j ]
        then
            rm -f $j
        fi
    done
}

# create_document dest head middle tail
create_document ()
{
    # create destination file
    cp $2 $1
    {
        for i in $opt_list
        do
            if [ ! -f $root_dir/$i/$tex_dir/$i/tex_errors ]
            then
                file=$i'_main.tex'
                if [ ! -e  $file ]
                then
                    create_empty_doc $i $file
                fi
                echo "\\input {$file}"
            fi
        done
    } >> $1

    # Special treatment for base...
    echo "\\input {base_middle.tex}" >> $1

    # Had to move dev-docs here...
    echo "\\input {dev_main.tex}" >> $1

    cat $3 >> $1

    {
        for i in $opt_list
        do
            file=$i'_appendix.tex'
            if [ ! -e  $file ]
            then
                touch $file
            fi
            echo "\\input {$file}"
        done
        echo "\\chapter{Différences des versions}"
        echo "\\input {changes.tex}"
    } >> $1

    {
    chapter=false
        for i in $opt_list
        do
            if [ -f $root_dir/$i/$tex_dir/$i/tex_errors ]
            then
                if [ $chapter = false ]
                then
                    echo "\\chapter{Tex errors}"
                    chapter=true
                fi
                file=$i'_main.tex'
                echo "\\input {$file}"
            fi
        done
    } >> $1

    cat $4 >> $1
}

create_local_conf ()
{
    > $1.add.conf
    > $1.files.$$
    > $1.deps.$$
    for i in base dev $opt_list
    do
        if [ -f $root_dir/$i/$tex_dir/$i/$i.conf ]
        then
            cat $root_dir/$i/$tex_dir/$i/$i.conf >> $1.add.conf
            name=`echo $i | tr '[a-z]' '[A-Z]'`
            echo -n "\$(${name}_CREATED_FILES) " >> $1.files.$$
            echo -n "\$(${name}_ADDITIONAL_DEPS) " >> $1.deps.$$
        fi
    done
    echo -n "CREATED_FILES := " >> $1.add.conf
    cat $1.files.$$ >> $1.add.conf
    echo >> $1.add.conf
    rm -f $1.files.$$
    echo -n "ADDITIONAL_DEPS := " >> $1.add.conf
    cat $1.deps.$$ >> $1.add.conf
    echo >> $1.add.conf
    rm -f $1.deps.$$
    echo 'foo:=$(warning additional dependencies: $(ADDITIONAL_DEPS))' >> $1.add.conf
}

create_changes()
{
    rm -f changes.tex
    for old_ver in $old_vers
    do
        ./gen_changes.pl $old_ver >>changes.tex
    done
}

if [ $# != 1 ]
then
    echo usage "$0 link|unlink|mkdoc|localconf|changes"
    exit 1
fi

if [ "$1" = "link" ]
then
    create_links
elif [ "$1" = "unlink" ]
then
    if [ -L base_main.tex ]
    then
        remove_links
    fi
elif [ "$1" = "changes" ]
then
    create_changes
elif [ "$1" = "mkdoc" ]
then
    create_changes
    create_links
    create_document doc.tex head.tex middle.tex tail.tex
    create_local_conf doc
elif [ "$1" = "localconf" ]
then
    create_links
    create_local_conf doc
else
    echo usage "$0 link|unlink|mkdoc|localconf"
fi
