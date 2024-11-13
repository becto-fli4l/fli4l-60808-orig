ifeq ($(DOCDIR),)
	DOCDIR := $(shell sh -c "if [ -d ../../../../../doc/doc/english/tex/doc ]; then echo '../../../../../doc/doc/english/tex/doc'; elif [ -d ../doc ]; then echo '../doc'; fi")
	ifeq ($(DOCDIR),)
		DOCDIRS := $(error "Please install the fli4l documentation package, which contains all necessary file to build ps/pdf/html documentation")
	endif
endif
include $(DOCDIR)/Make.conf
