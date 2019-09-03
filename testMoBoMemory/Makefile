#-------------------------------------------------------------------
#
# Copyright (c) 2009 Jabil Circuit, Inc., ALL RIGHTS RESERVED
#
# The information contained herein is confidential property of Jabil Circuit,
# Inc.. The use, copying, transfer, or disclosure of such information is 
# prohibited except by express written agreement with Jabil Circuit, Inc.
#
#-------------------------------------------------------------------

#-------------------------------------------------------------------
# \file Makefile
#
# \warning
# This Makefile was generated automatically from a template.
# 
# \author Maggie Tong
#
# \version r0.1.1
# \par ChangeLog:
# \verbatim
# rev 0.1.1 -
# \endverbatim
#-------------------------------------------------------------------

SUBDIRS= app driver
PWD= ${shell pwd;}
NAME= ${shell basename ${PWD};}
APP= app/bin/${NAME}
DIAG_HOME= /usr/local/diags
DOXYGEN= $(shell which doxygen 2>/dev/null)

DEBUG= -D__DEBUG__
export DEBUG

all: subdirs

subdirs:
	for i in ${SUBDIRS}; do \
		$(MAKE) -C $$i || exit 1; \
	done


install:
	for i in $(SUBDIRS); do \
		cd $$i; make install; cd -;\
	done

uninstall:
	for i in $(SUBDIRS); do \
		cd $$i; make uninstall; cd -;\
	done

binary: 
	for i in app; do \
		cd $$i; make; cd -;\
	done

clean:
	for i in $(SUBDIRS); do \
		cd $$i; make clean; cd -;\
	done

.PHONY:doc
doc:
	cd doc;\
	make doc

