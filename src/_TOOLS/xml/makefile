# $Id$
#******************************************************************************
# to execute this makefile it's required to have xsltproc/libxslt1.1 installed
#
# 2014.03.03 Carsten Spiess
# - created 

ifneq ($(PKG),)
#******************************************************************************
# choose XSLT processor
# XSLT_PROC can be either libxslt, xalan, saxon (XSLT 1.0) or saxonb (XSLT 2.0)
# defaults to libxsl
ifndef XSLT_PROC
XSLT_PROC       := libxslt
endif
XSLT_FLAGS      :=
 
ifeq ($(XSLT_PROC),libxslt)
# definitions for libxslt
XSLT            := xsltproc
XSLT_OUT        := -o
else # libxslt
ifeq ($(XSLT_PROC),xalan)
# definitions for xalan
XSLT            := xalan
XSLT_OUT        := -out
XSLT_IN         := -in
XSLT_XSL        := -xsl
else # xalan
ifeq ($(XSLT_PROC),saxon)
# definitions for saxon 
XSLT            := saxon-xslt
XSLT_OUT        := -o
else # saxon
ifeq ($(XSLT_PROC),saxonb)
# definitions for saxonb
XSLT            := saxonb-xslt
XSLT_FLAGS      += -strip:all
XSLT_OUT        := -o
else # saxonb
error no valid XSLT_PROC defined
endif # saxonb
endif # saxon
endif # xalan
endif # libxslt

#******************************************************************************
# package dir
PKG_DIR         := ../../packages/$(PKG)
#******************************************************************************
# rrd-graph dirs
XML_DIR         := $(PKG_DIR)/opt/srv/www/xml
INC_DIR         := $(PKG_DIR)/opt/srv/www/include
CGI_DIR         := $(PKG_DIR)/opt/srv/www/admin

#******************************************************************************
# rrd-graph files
RRD_INC_XSL     := rrd-graph.xsl
HWMON_INC_XSL   := hwmon-inc.xsl
RRD_CGI_XSL     := rrd-cgi.xsl

RRD_XML         := $(notdir $(wildcard $(XML_DIR)/*.xml))
RRD_INC         := $(addprefix $(INC_DIR)/,$(patsubst %.xml,rrd-%.inc,$(RRD_XML)))
HWMON_INC       := $(addprefix $(INC_DIR)/,$(patsubst %.xml,hwmon-%.inc,$(RRD_XML)))
RRD_CGI         := $(addprefix $(CGI_DIR)/,$(patsubst %.xml,_%.cgi,$(RRD_XML)))

#******************************************************************************
# common target
.PHONY: all
all: rrd-inc hwmon-inc rrd-cgi

#******************************************************************************
# create rrd graph includes
.PHONY: rrd-inc
rrd-inc: $(RRD_INC)

# create hwmon includes
.PHONY: hwmon-inc
hwmon-inc: $(HWMON_INC)

#create rrd cgi
.PHONY: rrd-cgi
rrd-cgi: $(RRD_CGI)

#******************************************************************************
# transform
$(INC_DIR)/rrd-%.inc: $(XML_DIR)/%.xml $(RRD_INC_XSL) makefile
	$(XSLT) $(XSLT_FLAGS) $(XSLT_OUT) $@ $(XSLT_XSL) $(RRD_INC_XSL) $(XSLT_IN) $< 

$(INC_DIR)/hwmon-%.inc: $(XML_DIR)/%.xml $(HWMON_INC_XSL) makefile
	$(XSLT) $(XSLT_FLAGS) $(XSLT_OUT) $@ $(XSLT_XSL) $(HWMON_INC_XSL) $(XSLT_IN) $< 

$(CGI_DIR)/_%.cgi: $(XML_DIR)/%.xml $(RRD_CGI_XSL) makefile
	$(XSLT) $(XSLT_FLAGS) $(XSLT_OUT) $@ $(XSLT_XSL) $(RRD_CGI_XSL) $(XSLT_IN) $< 

else # $(PKG)
#******************************************************************************
# packages
PACKAGES        := $(notdir $(wildcard ../../packages/*))

#******************************************************************************
# targets
.PHONY: all
all: rrd-inc hwmon-inc rrd-cgi 
	
.PHONY: rrd-inc
rrd-inc: $(PACKAGES)

.PHONY: hwmon-inc
hwmon-inc: hwsupp

.PHONY: rrd-cgi
rrd-cgi: hwsupp

.PHONY: $(PACKAGES)
$(PACKAGES):
	$(MAKE) PKG=$@ $(MAKECMDGOALS)

endif # $(PKG)
# _oOo_
