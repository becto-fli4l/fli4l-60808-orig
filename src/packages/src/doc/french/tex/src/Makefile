ifeq ($(DOCDIR),)
	DOCDIR := $(shell sh -c "if [ -d ../../../../../doc/doc/french/tex/doc ]; then echo '../../../../../doc/doc/french/tex/doc'; elif [ -d ../doc ]; then echo '../doc'; fi")
	ifeq ($(DOCDIR),)
		DOCDIRS := $(error "Please install the fli4l documentation package, which contains all necessary files to build ps/pdf/html documentation")
	endif
endif
include $(DOCDIR)/Make.conf
