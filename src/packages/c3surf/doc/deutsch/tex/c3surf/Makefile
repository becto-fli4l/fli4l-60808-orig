ifeq ($(DOCDIR),)
	DOCDIR := $(shell sh -c "if [ -d ../../../../../doc/doc/deutsch/tex/doc ]; then echo '../../../../../doc/doc/deutsch/tex/doc'; elif [ -d ../doc ]; then echo '../doc'; fi")
	ifeq ($(DOCDIR),)
		DOCDIRS := $(error "Please install the fli4l documentation package, which contains all necessary files to build ps/pdf/html documentation")
	endif
endif

# this functions in Distro; still needs to be adapted to make it function in SVN too
# fs 2010 for svn I created the c3surf-version.tex manualy
#C3SURFVER=`cat ../../../../c3surf-version.txt`
#DUMMY := $(shell sh -c 'printf "\\\\newcommand{\\\\cIIIsurfver}{%s}\n" $(C3SURFVER) >./c3surf-version.tex')

include $(DOCDIR)/Make.conf
